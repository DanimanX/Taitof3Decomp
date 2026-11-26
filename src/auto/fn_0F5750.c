/* ROM 0x0F5750: enemy tick (A0 = object).
 * Handles bomb-hit check (0x0CEF0E), HP death (5-way shot 0xD9D9E + 0xDC486),
 * movement script (0x0CF1E4 / 0x0CF4B8), off-screen cull (0x0CF85A),
 * state dispatch (0x0F5A0C[0..4]), script/collision/blink/draw, and
 * aimed shot spawn (0x0D9092). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0CF7FE: clear bit 7 and set bit 1 of +0x07 when +0x3A is in [-64, 335]. */
static void sub_0CF7FE(const f3mem_t *m, uint32_t obj)
{
    uint8_t f7 = rd8(m, obj + 0x07u);
    if ((f7 & 0x80u) == 0u)
        return;
    int16_t x = (int16_t)rd16(m, obj + 0x3Au);
    if (x < -0x40 || x > 0x014F)
        return;
    wr8(m, obj + 0x07u, (uint8_t)((f7 & ~0x80u) | 0x02u));
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

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D09B8 player check */
static int16_t check_player_0D09B8(const f3mem_t *m, uint32_t obj, uint32_t a1,
                                   uint32_t player, uint16_t *out_dx, uint16_t *out_dy,
                                   uint32_t *d0_io, uint32_t *d1_io)
{
    if ((int16_t)rd16(m, player + 0x4Cu) < 0)
        return -1;

    uint32_t a4 = rd32(m, obj + 0x68u) + 2u;
    int16_t d3w = (int16_t)((int16_t)rd16(m, obj + 0x18u) - (int16_t)rd16(m, player + 0x18u)
                            + (int16_t)rd16(m, a4) - 0x0F);
    int16_t a3w = d3w;
    if (d3w < 0)
        d3w = (int16_t)(-d3w);
    if ((uint16_t)d3w > rd16(m, a1 + 0x0Au))
        return -1;

    a4 -= 2u;
    int16_t d2w = (int16_t)((int16_t)rd16(m, obj + 0x14u) - (int16_t)rd16(m, player + 0x14u)
                            + (int16_t)rd16(m, a4) - 0x17);
    int16_t a4w = d2w;
    if (d2w < 0) {
        d2w = (int16_t)(-d2w);
        if ((uint16_t)d2w > rd16(m, a1 + 0x0Au))
            return -1;
        if ((uint16_t)d2w < rd16(m, a1 + 0x08u) && (uint16_t)d3w < rd16(m, a1 + 0x08u))
            return -1;
    } else {
        if ((uint16_t)d2w > rd16(m, a1 + 0x0Au))
            return -1;
        int16_t d0w = (int16_t)(rd16(m, a1 + 0x08u) - 0x1Eu);
        if (d2w < d0w && (uint16_t)d3w < rd16(m, a1 + 0x08u))
            return -1;
    }

    *out_dx = (uint16_t)d2w;
    *out_dy = (uint16_t)d3w;

    uint16_t ang = Obj_Atan256(m, a3w, a4w);
    *d0_io = setw(*d0_io, ang);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);
    *d1_io = setw(*d1_io, lo);
    if (lo > hi) {
        if (ang >= lo || ang <= hi)
            return (int16_t)ang;
    } else {
        if (ang >= lo && ang <= hi)
            return (int16_t)ang;
    }
    return -1;
}

/* ROM 0x0D09B8 with D0/D1 register tracking. */
static void aim_players_regs(const f3mem_t *m, uint32_t obj, uint32_t a1,
                             uint32_t *d0_io, uint32_t *d1_io)
{
    uint32_t a2 = rd32(m, obj + 0x68u);
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + (int16_t)rd16(m, a2));
    *d0_io = setw(*d0_io, (uint16_t)x);
    if (x < 0x0028 || x > 0x0127)
        return;
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + (int16_t)rd16(m, a2 + 2u));
    *d0_io = setw(*d0_io, (uint16_t)y);
    if (y < 0x0010 || y > 0x00DF)
        return;

    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t ang1 = check_player_0D09B8(m, obj, a1, 0x0041B190u, &dx1, &dy1, d0_io, d1_io);
    int16_t ang2 = check_player_0D09B8(m, obj, a1, 0x0041B21Au, &dx2, &dy2, d0_io, d1_io);

    int16_t chosen = ang2;
    if (ang1 < 0) {
        if (ang2 < 0) {
            *d0_io = setw(*d0_io, (uint16_t)ang2);
            return;
        }
    } else if (ang2 >= 0) {
        uint16_t m1 = (dx1 <= dy1) ? dx1 : dy1;
        uint16_t m2 = (dx2 <= dy2) ? dx2 : dy2;
        if (m2 >= m1)
            chosen = ang1;
    } else {
        chosen = ang1;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    uint16_t idx = (uint16_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    uint16_t d1w = (uint16_t)(rd16(m, a1 + 0x10u) + rd16(m, 0x000D0D42u + (uint32_t)idx * 2u));
    uint16_t rng = Obj_Next4Way(m);
    *d1_io = setw(*d1_io, d1w);
    if ((int16_t)d1w > (int16_t)rng) {
        uint16_t cnt = rd16(m, a1 + 0x12u);
        *d0_io = setw(*d0_io, cnt);
        wr8(m, obj + 0x23u, (uint8_t)cnt);
    } else {
        *d0_io = setw(*d0_io, rng);
    }
}

int Fn_0F5750(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x0F5750 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x0F5768 */
            d0 = 0x00000FA0u;                           /* 0x0F59FA */
            Obj_RelayTimer(m, d0);                      /* 0x0F5A00 */
            Obj_0CEF0E(m, obj);                         /* 0x0F5A06 */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x0F5774 */
        d0 = 0x00000FA0u;                               /* 0x0F586E */
        Obj_0D0E04(m, obj, d0);                         /* 0x0F5874 */
        uint32_t x = rd32(m, obj + 0x14u) + 0x00180000u; /* 0x0F587A */
        uint32_t y = rd32(m, obj + 0x18u) + 0x00100000u; /* 0x0F5884 */
        uint16_t z = rd16(m, obj + 0x60u);              /* 0x0F588E */
        static const uint16_t angs[5] = { 0x008Cu, 0x00A0u, 0x00B4u, 0x00C8u, 0x00DCu };
        for (int i = 0; i < 5; i++) {
            d0 = Obj_Spawn(m, obj, 0x000D9D9Eu, 9u, 2u, 0u);
            if ((int32_t)d0 < 0) {
                Obj_StartDying(m, obj);
                r->d[0] = d0;
                r->d[1] = d1;
                r->a[1] = a1;
                return 0;
            }
            a1 = d0;
            wr32(m, a1 + 0x14u, x);
            wr32(m, a1 + 0x18u, y);
            wr16(m, a1 + 0x60u, z);
            wr16(m, a1 + 0x58u, angs[i]);
        }
        goto death_fx;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)        /* 0x0F577C */
        goto death_fx;

    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F5786 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F578A */

    {
        uint8_t f7 = rd8(m, obj + 0x07u);               /* 0x0F578E */
        wr8(m, obj + 0x07u, (uint8_t)(f7 & ~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)            /* 0x0F5798 */
                return -1;
        }
    }

    if (rd8(m, obj + 0x20u) != 0u) {                    /* 0x0F579E */
        a1 = 0x000F51F2u;                               /* 0x0F57A6 */
        if (ObjScript_CF4B8(m, obj, a1) < 0)            /* 0x0F57AA */
            return -1;
    }

    Obj_FlagOffscreen(m, obj);                          /* 0x0F57B0 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {          /* 0x0F57B6 */
        Obj_StartDying(m, obj);                         /* 0x0F57BC */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));         /* 0x0F57C2 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));         /* 0x0F57C8 */

    d0 = setw(d0, (uint16_t)rd8(m, obj + 0x06u));       /* 0x0F57CE..0x0F57D0 */
    switch ((uint16_t)d0) {                             /* 0x0F57D4 -> 0x0F5A0C */
    case 0u:                                            /* 0x0F5A20 */
        sub_0CF7FE(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u)
            wr8(m, obj + 0x06u, 1u);
        break;
    case 1u:                                            /* 0x0F5A36 */
        break;
    case 2u:                                            /* 0x0F5A38 */
        if ((int16_t)rd16(m, obj + 0x68u) != 0) {
            a1 = 0x000F53F6u;
            Obj_StartBehaviour(m, obj, a1);
            a1 += 8u;
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 3u:                                            /* 0x0F5A52 */
        if ((int16_t)rd16(m, obj + 0x68u) != 0) {
            a1 = 0x000F544Au;
            Obj_StartBehaviour(m, obj, a1);
            a1 += 8u;
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 4u:                                            /* 0x0F5A6C */
        if ((int16_t)rd16(m, obj + 0x68u) != 0) {
            a1 = 0x000F549Eu;
            Obj_StartBehaviour(m, obj, a1);
            a1 += 8u;
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    }

    step_script_regs(m, obj, &d0, &a1);                 /* 0x0F57DC */
    add_coll_regs(m, obj, &a1);                         /* 0x0F57E2 */
    Obj_BlinkColour(m, obj);                            /* 0x0F57E8 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0F57EE */

    if (rd8(m, obj + 0x23u) != 0u) {                    /* 0x0F57F4 */
        uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u); /* 0x0F57FC */
        wr8(m, obj + 0x23u, c);
        if (c != 0u) {
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    {
        uint32_t saved_68 = rd32(m, obj + 0x68u);       /* 0x0F5804 */
        wr32(m, obj + 0x68u, 0x000F54B6u);              /* 0x0F5808 */
        a1 = 0x000F51F2u;                               /* 0x0F5810 */
        aim_players_regs(m, obj, a1, &d0, &d1);         /* 0x0F5814 */
        wr32(m, obj + 0x68u, saved_68);                 /* 0x0F581A */
    }

    if (rd8(m, obj + 0x23u) != 0u) {                    /* 0x0F581E */
        d0 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u); /* 0x0F5826..0x0F5838 */
        if ((int32_t)d0 >= 0) {                         /* 0x0F5844 */
            a1 = d0;                                    /* 0x0F5848 */
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));  /* 0x0F584A */
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0018u)); /* 0x0F5850 */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));  /* 0x0F5856 */
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u)); /* 0x0F585C */
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));  /* 0x0F5862 */
            wr16(m, a1 + 0x10u, 0u);                    /* 0x0F5868 */
        }
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_fx:
    d0 = Obj_Spawn(m, obj, 0x000DC486u, 3u, 2u, 0u);    /* 0x0F59AA..0x0F59BC */
    if ((int32_t)d0 >= 0) {                             /* 0x0F59C8 */
        a1 = d0;
        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0018u));
        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
        wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
        wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
    }
    Obj_StartDying(m, obj);                             /* 0x0F59F4 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
