/* ROM 0x10EDA0: enemy variant tick (A0 = object).
 * When +0x06 != 0 (landing/impact state), steps script (0x0D0690), dies via
 * 0x0CEE74 if +0x4C == 0, else scrolls by A5+0x4224/4228 and draws (0x001282).
 * When +0x06 == 0, handles bomb hit (A5+0x1D58 bit 1 -> 0x0D0E30 + 0x0CEF0E),
 * HP <= 0 (0x0D0E04 -> spawn 0x10F2EE + 0x0CEE74), A5-0x144B bit 5 (spawn
 * 0x10F2EE + 0x0CEE74), otherwise advances (+0x14, +0x18) by (+0x50, +0x54)
 * minus scroll, dies if Y > 0xE8, tests terrain (0x0C6632) at (+0x14+0x3A,
 * +0x18+0x42): on clear adds to collision lists (0x0D0DC2), blinks colour
 * (0x0CBE34), draws (0x001282), and adds gravity 0x36B0 to +0x54; on hit
 * spawns 0x10F2EE, starts behaviour +0x44 (0x0D0678), offsets pos by
 * (+0x3C, +0x40), clears +0x1D, draws, and sets +0x06 = 1. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0C6632 wrapper tracking D0/D1 register state. */
static uint8_t terrain_test_regs(const f3mem_t *m, int16_t x, int16_t y,
                                 uint32_t *d0_io, uint32_t *d1_io)
{
    uint8_t hit = (uint8_t)Terrain_TestPoint(m, x, y);
    if (rd8(m, F3_A5 + 0x1D1Cu) != 0u) {
        *d0_io = (*d0_io & 0xFFFFFF00u);
        return 0u;
    }
    uint32_t d0 = (uint32_t)(((uint16_t)((uint16_t)x + rd16(m, F3_A5 + 0x0918u)) >> 3) & 0x3FFu);
    uint32_t d1 = (uint32_t)(((uint16_t)((uint16_t)y + rd16(m, F3_A5 + 0x091Au)) >> 3) & 0x3Fu);
    d1 = (uint32_t)((int32_t)(int16_t)d1 * 0x280);
    d0 += d1;
    *d1_io = d1;
    *d0_io = (d0 & 0xFFFFFF00u) | (hit ? 0xFFu : 0x00u);
    return hit;
}

/* ROM 0x0D0DC2 with A1 register tracking. */
static void add_coll_regs(const f3mem_t *m, uint32_t obj, uint32_t *a1_io)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_io = a1;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
}

int Fn_10EDA0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if (rd8(m, obj + 0x06u) != 0u) {                    /* 0x10EDA0 */
        step_script_regs(m, obj, &d0, &a1);              /* 0x10EF12 */
        if (rd16(m, obj + 0x4Cu) == 0u) {                /* 0x10EF18 */
            Obj_StartDying(m, obj);                      /* 0x10EF1C */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
        d0 = rd32(m, F3_A5 + 0x4224u);                   /* 0x10EF22 */
        wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0); /* 0x10EF26 */
        d0 = rd32(m, F3_A5 + 0x4228u);                   /* 0x10EF2A */
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0); /* 0x10EF2E */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        draw_sprite_regs(m, r);                          /* 0x10EF32 */
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x10EDA8 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);       /* 0x10EDB0 */
        if (x >= -0x000A && x <= 0x011C) {               /* 0x10EDB6..0x10EDBE */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));    /* 0x10EDC0 */
            d0 = 0x000000C8u;                            /* 0x10EE94 */
            Obj_RelayTimer(m, d0);                       /* 0x10EE9A */
            a1 = rd32(m, F3_A5 + 0x1D50u);
            Obj_0CEF0E(m, obj);                          /* 0x10EEA0 -> 0x0CEF0E */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {            /* 0x10EDCC */
        d0 = 0x000000C8u;                                /* 0x10EE3A */
        Obj_0D0E04(m, obj, d0);                          /* 0x10EE40 */
        uint8_t k = rd8(m, obj + 0x35u);
        if (k == 1u)
            a1 = rd32(m, rd32(m, obj + 0x36u) + 8u);
        else if (k == 2u)
            a1 = rd32(m, obj + 0x36u);
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)         /* 0x10EDD4 */
        goto death_spawn;

    d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u); /* 0x10EDDE..0x10EDE2 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);      /* 0x10EDE6 */
    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u); /* 0x10EDEA..0x10EDEE */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);      /* 0x10EDF2 */
    if ((int16_t)rd16(m, obj + 0x18u) > 0x00E8) {         /* 0x10EDF6 */
        Obj_StartDying(m, obj);                           /* 0x10EDFC */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    d0 = setw(d0, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x3Au))); /* 0x10EE02..0x10EE06 */
    d1 = setw(d1, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x42u))); /* 0x10EE0A..0x10EE0E */
    if (terrain_test_regs(m, (int16_t)(uint16_t)d0, (int16_t)(uint16_t)d1, &d0, &d1) != 0u) { /* 0x10EE12 */
        d0 = Obj_Spawn(m, obj, 0x0010F2EEu, 9u, 2u, 0u);  /* 0x10EEA6..0x10EEB8 */
        if ((int32_t)d0 >= 0) {                           /* 0x10EEC2 */
            a1 = d0;                                      /* 0x10EEC8 */
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));    /* 0x10EECA */
            d0 = setw(d0, rd16(m, obj + 0x3Au));          /* 0x10EED0 */
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + (uint16_t)d0)); /* 0x10EED4 */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));    /* 0x10EED8 */
            d0 = setw(d0, rd16(m, obj + 0x3Eu));          /* 0x10EEDE */
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + (uint16_t)d0)); /* 0x10EEE2 */
        }
        a1 = rd32(m, obj + 0x44u);                        /* 0x10EEE6 */
        Obj_StartBehaviour(m, obj, a1);                   /* 0x10EEEA */
        a1 += 8u;
        d0 = setw(d0, rd16(m, obj + 0x3Cu));              /* 0x10EEF0 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d0)); /* 0x10EEF4 */
        d0 = setw(d0, rd16(m, obj + 0x40u));              /* 0x10EEF8 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0)); /* 0x10EEFC */
        wr8(m, obj + 0x1Du, 0u);                          /* 0x10EF00 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        draw_sprite_regs(m, r);                           /* 0x10EF04 */
        wr8(m, obj + 0x06u, 1u);                          /* 0x10EF0A */
        return 0;
    }

    add_coll_regs(m, obj, &a1);                           /* 0x10EE1E */
    Obj_BlinkColour(m, obj);                              /* 0x10EE24 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                               /* 0x10EE2A */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x36B0u); /* 0x10EE30 */
    return 0;

death_spawn:
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    draw_sprite_regs(m, r);                               /* 0x10EE46 */
    d0 = Obj_Spawn(m, obj, 0x0010F2EEu, 9u, 2u, 0u);      /* 0x10EE4C..0x10EE5E */
    if ((int32_t)d0 >= 0) {                               /* 0x10EE68 */
        r->a[1] = d0;                                     /* 0x10EE70 */
        wr32(m, r->a[1] + 0x14u, rd32(m, obj + 0x14u));   /* 0x10EE72 */
        d0 = setw(d0, rd16(m, obj + 0x3Au));              /* 0x10EE78 */
        wr16(m, r->a[1] + 0x14u, (uint16_t)(rd16(m, r->a[1] + 0x14u) + (uint16_t)d0)); /* 0x10EE7C */
        wr32(m, r->a[1] + 0x18u, rd32(m, obj + 0x18u));   /* 0x10EE80 */
        d0 = setw(d0, rd16(m, obj + 0x3Eu));              /* 0x10EE86 */
        wr16(m, r->a[1] + 0x18u, (uint16_t)(rd16(m, r->a[1] + 0x18u) + (uint16_t)d0)); /* 0x10EE8A */
    }
    r->d[0] = d0;
    Obj_StartDying(m, obj);                               /* 0x10EE6A / 0x10EE8E */
    return 0;
}
