/* ROM 0x10BE6C: boss/large-enemy state machine tick (A0 = object).
 * When screen flag A5+0x1D58 bit 1 is set and X in [-10, 0x11C], resets
 * +0x1D from +0x4F, clears +0x06, adds 0x12C score via 0x0D0E30, sets sprite
 * 0x065710 at (-3, +8) and jumps to 0x0CBB2C. When +0x24 <= 0 or pause/bomb
 * bit (A5-0x144B bit 5) is set, spawns 0x0DC456 and starts dying (0x0CEE74).
 * Otherwise scrolls +0x3A/+0x3E, runs 0x0CF85A (dying if offscreen bit 3 set),
 * copies +0x3A/+0x3E to +0x14/+0x18, dispatches +0x06 via table 0x10BF82
 * (1: arm onscreen 0x0CF7FE, 2: aim 0x0D07B4 + behaviour 0x0D0678, 3: cooldown
 * + aim + spawn 0x10C242), steps script (0x0D0690), appends collision lists
 * (0x0D0DC2), updates hit flash (0x0CBE34), and draws sprite (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02 */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t idx = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)idx * 2u);
}

/* ROM 0x0D07E4 + 0x0D08A0: test one player record against aim cone at a1 */
static int16_t sub_0D07E4_player(const f3mem_t *m, uint32_t obj, uint32_t a1,
                                 uint32_t player, uint16_t *out_dx, uint16_t *out_dy)
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

    uint16_t ang = Obj_Atan256(m, a4w, a3w);
    uint16_t lo = rd16(m, a1 + 0x0Cu);
    uint16_t hi = rd16(m, a1 + 0x0Eu);
    if (lo > hi) {
        if (ang >= lo || ang <= hi)
            return (int16_t)ang;
    } else {
        if (ang >= lo && ang <= hi)
            return (int16_t)ang;
    }
    return -1;
}

/* ROM 0x0D07B4: screen-bounds check + aim cone over both players */
static void sub_0D07B4(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint32_t a2 = rd32(m, obj + 0x68u);
    int16_t x = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, a2));
    if (x < 0x0028 || x > 0x0127)
        return;
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, a2 + 2u));
    if (y < 0x0010 || y > 0x00DF)
        return;

    uint16_t dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
    int16_t ang1 = sub_0D07E4_player(m, obj, a1, 0x0041B190u, &dx1, &dy1);
    int16_t ang2 = sub_0D07E4_player(m, obj, a1, 0x0041B21Au, &dx2, &dy2);

    int16_t chosen = ang2;
    if (ang1 < 0) {
        if (ang2 < 0)
            return;
    } else if (ang2 >= 0) {
        uint16_t m1 = (dx1 <= dy1) ? dx1 : dy1;
        uint16_t m2 = (dx2 <= dy2) ? dx2 : dy2;
        if (m2 >= m1)
            chosen = ang1;
    } else {
        chosen = ang1;
    }

    wr16(m, obj + 0x42u, (uint16_t)chosen);
    int16_t chance = (int16_t)(rd16(m, a1 + 0x10u) + sub_0D0C8C(m));
    int16_t rng = (int16_t)Obj_Next4Way(m);
    if (chance > rng)
        wr8(m, obj + 0x23u, (uint8_t)rd16(m, a1 + 0x12u));
}

static void draw_sprite_with_regs(const f3mem_t *m, f3regs_t *r, uint32_t obj,
                                  uint32_t d0, uint32_t d1)
{
    Obj_DrawSprite(m, obj);
    uint32_t a2 = rd32(m, obj + 0x10u);
    r->a[1] = rd32(m, SPR_CURSOR);
    if (rd32(m, a2) == 0x00010001u) {
        d0 = ((uint32_t)rd16(m, obj + 0x14u) << 16) | rd16(m, obj + 0x18u);
        d0 &= 0x0FFF0FFFu;
        d0 = setw(d0, rd16(m, a2 + 6u));
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        d1 = setw(d1, (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u));
    } else {
        uint16_t cols_m1 = (uint16_t)(rd16(m, a2) - 1u);
        d0 = 0x0000FFFFu;
        d1 = ((uint32_t)cols_m1 << 16) | 0xFFFFu;
    }
    r->d[0] = d0;
    r->d[1] = d1;
}

int Fn_10BE6C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    /* 0x10BE6C */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {                    /* 0x10BE74..0x10BE82 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x10BE84 */
            wr8(m, obj + 0x06u, 0u);                        /* 0x10BF5C */
            Obj_RelayTimer(m, 0x0000012Cu);                 /* 0x10BF60..0x10BF66 */
            wr32(m, obj + 0x10u, 0x00065710u);              /* 0x10BF6C */
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 3u)); /* 0x10BF74 */
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 8u)); /* 0x10BF78 */
            Obj_ExplodeTick(m, obj);                        /* 0x10BF7C */
            r->a[1] = rd32(m, F3_A5 + 0x1D50u);
            r->d[0] = 0x0000012Cu;
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* 0x10BE90..0x10BE94 */
        Obj_RelayTimer(m, 0x0000012Cu);                     /* 0x10BEF6..0x10BEFC */
        d0 = 0x0000012Cu;
        r->a[1] = rd32(m, F3_A5 + 0x1D50u);
        goto death_spawn;
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)            /* 0x10BE98..0x10BE9E */
        goto death_spawn;

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u)); /* 0x10BEA2..0x10BEA6 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u)); /* 0x10BEAA..0x10BEAE */
    Obj_FlagOffscreen(m, obj);                              /* 0x10BEB2 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {              /* 0x10BEB8..0x10BEBE */
        wr8(m, obj + 0x06u, 0u);                            /* 0x10BF52 */
        Obj_StartDying(m, obj);                             /* 0x10BF56 */
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));             /* 0x10BEC2 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));             /* 0x10BEC8 */

    {
        uint8_t state = rd8(m, obj + 0x06u);                /* 0x10BECE..0x10BED0 */
        d0 = setw(d0, state);
        switch (state) {                                    /* 0x10BED4 table 0x10BF82 */
        case 1u:
            Obj_ArmOnScreen(m, obj);                        /* 0x10BF92 */
            if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {      /* 0x10BF98..0x10BF9E */
                wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x10BFA2 */
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));  /* 0x10BFA8 */
                wr8(m, obj + 0x06u, 2u);                    /* 0x10BFAE */
            }
            break;
        case 2u:
            sub_0D07B4(m, obj, 0x0010C05Cu);                /* 0x10BFB6..0x10BFBA */
            if (rd8(m, obj + 0x23u) != 0u) {                /* 0x10BFC0..0x10BFC4 */
                Obj_StartBehaviour(m, obj, 0x0010C0E4u);    /* 0x10BFC8..0x10BFCC */
                wr8(m, obj + 0x23u, 0u);                    /* 0x10BFD2 */
                wr8(m, obj + 0x06u, 3u);                    /* 0x10BFD6 */
            }
            break;
        case 3u:
            if ((int16_t)rd16(m, obj + 0x68u) < 0) {        /* 0x10BFDE..0x10BFE2 */
                uint8_t cnt = rd8(m, obj + 0x23u);          /* 0x10BFE6 */
                if (cnt != 0u) {
                    cnt = (uint8_t)(cnt - 1u);              /* 0x10BFEE */
                    wr8(m, obj + 0x23u, cnt);
                    if (cnt != 0u)                          /* 0x10BFF2 */
                        break;
                }
                uint32_t saved68 = rd32(m, obj + 0x68u);    /* 0x10BFF6 */
                wr32(m, obj + 0x68u, 0x0010C15Cu);          /* 0x10BFFA */
                sub_0D07B4(m, obj, 0x0010C094u);            /* 0x10C002..0x10C006 */
                wr32(m, obj + 0x68u, saved68);              /* 0x10C00C */
                if (rd8(m, obj + 0x23u) != 0u) {            /* 0x10C010..0x10C014 */
                    uint32_t child = Obj_Spawn(m, obj, 0x0010C242u, 3u, 2u, 0u); /* 0x10C018..0x10C02A */
                    d0 = child;
                    if ((int32_t)child >= 0) {              /* 0x10C034..0x10C036 */
                        uint32_t a1 = child;
                        wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u)); /* 0x10C03C */
                        wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x19u)); /* 0x10C042 */
                        wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u)); /* 0x10C048 */
                        wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x28u)); /* 0x10C04E */
                        wr16(m, a1 + 0x58u, 0x004Au);       /* 0x10C054 */
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    Obj_StepScript(m, obj);                                 /* 0x10BEDC */
    Obj_0D0DC2(m, obj);                                     /* 0x10BEE2 */
    Obj_BlinkColour(m, obj);                                /* 0x10BEE8 */
    draw_sprite_with_regs(m, r, obj, d0, d1);               /* 0x10BEEE */
    return 0;                                               /* 0x10BEF4 */

death_spawn:
    wr8(m, obj + 0x06u, 0u);                                /* 0x10BF02 */
    {
        uint32_t child = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u); /* 0x10BF06..0x10BF18 */
        d0 = child;
        if ((int32_t)child >= 0) {                          /* 0x10BF22..0x10BF24 */
            uint32_t a1 = child;
            r->a[1] = a1;
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));      /* 0x10BF2C */
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x18u)); /* 0x10BF32 */
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));      /* 0x10BF38 */
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x28u)); /* 0x10BF3E */
            wr16(m, a1 + 0x58u, 0u);                        /* 0x10BF44 */
            wr16(m, a1 + 0x60u, 0u);                        /* 0x10BF48 */
        }
    }
    Obj_StartDying(m, obj);                                 /* 0x10BF4C */
    r->d[0] = d0;
    return 0;
}
