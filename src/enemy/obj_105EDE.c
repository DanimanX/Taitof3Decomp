#include "obj_105EDE.h"
#include "../auto/fn_helpers.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

/* ROM 0x0D0C8C: difficulty-scaled fire threshold from table 0x0D0D02. */
static uint16_t sub_0D0C8C(const f3mem_t *m)
{
    uint8_t idx = (uint8_t)(rd8(m, F3_A5 + 0x087Fu) >> 3);
    return rd16(m, 0x000D0D02u + (uint32_t)idx * 2u);
}

/* ROM 0x0D07E4 + 0x0D08A0: test one player record against aim cone at A1. */
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

/* ROM 0x0D07B4: screen-bounds check + aim cone over both players. */
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

/* ROM 0x0D0BBC: turn obj+0x08 toward obj+0x42 by step *(a1+4). */
static void sub_0D0BBC(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t target = rd16(m, obj + 0x42u);
    int16_t diff = (int16_t)(target - rd16(m, obj + 0x08u));
    if (diff < 0)
        diff = (int16_t)(diff + 0x0168);

    uint16_t step_w = rd16(m, a1 + 0x04u);
    uint32_t step_l = rd32(m, a1 + 0x04u);
    if ((uint16_t)diff > 0x00B4u) {
        uint16_t ccw = (uint16_t)(0x0168 - diff);
        if (ccw <= step_w) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        uint32_t v = rd32(m, obj + 0x08u) - step_l;
        wr32(m, obj + 0x08u, v);
        if ((int32_t)v < 0)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) + 0x0168u));
    } else {
        if ((uint16_t)diff <= step_w) {
            wr16(m, obj + 0x08u, target);
            return;
        }
        wr32(m, obj + 0x08u, rd32(m, obj + 0x08u) + step_l);
        if (rd16(m, obj + 0x08u) >= 0x0168u)
            wr16(m, obj + 0x08u, (uint16_t)(rd16(m, obj + 0x08u) - 0x0168u));
    }
}

/* ROM 0x0CFD66: apply frame record A1 + table 0x0CFD80[d0]. */
void sub_0CFD66(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)rd16(m, 0x000CFD80u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    a1 += (uint32_t)off;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D0384: install animation script via table 0x0D03A8[d0]. */
static void sub_0D0384(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0)
{
    int16_t off = (int16_t)rd16(m, 0x000D03A8u + (uint32_t)(int32_t)(int16_t)d0 * 2u);
    uint32_t ptr = rd32(m, a1 + (uint32_t)off);
    Obj_StartBehaviour(m, obj, ptr);
}

/* ROM 0x1065B6: choose next +0x06 state and clear +0x22. Returns -1 when D0
 * is left alone, else the word 0x0D0E3A left in D0 (both branches at 0x106606
 * and 0x106622 compare that word). */
int sub_1065B6(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x22u, 0u);
    if (rd16(m, obj + 0x50u) == 0u) {
        wr8(m, obj + 0x06u, 3u);
        return -1;
    }
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < 0x003C) {
        wr8(m, obj + 0x06u, 6u);
        return -1;
    } else if (x < 0x0078) {
        wr8(m, obj + 0x06u, 4u);
        uint16_t r = Obj_Next4Way(m);                       /* 0x106606 jsr 0x0d0e3a */
        if (r <= 0x0032u)
            wr8(m, obj + 0x06u, 6u);
        return (int)(r & 0xFFFFu);
    } else if (x <= 0x00DC) {
        if (rd8(m, obj + 0x06u) == 2u)
            wr8(m, obj + 0x06u, 4u);
        return -1;
    } else {
        wr8(m, obj + 0x06u, 3u);
        uint16_t r = Obj_Next4Way(m);                       /* 0x106622 jsr 0x0d0e3a */
        if (r >= 0x0032u) {
            wr8(m, obj + 0x06u, 5u);
            if (r >= 0x0064u)
                wr8(m, obj + 0x06u, 4u);
        }
        return (int)(r & 0xFFFFu);
    }
}

/* ROM 0x1061A4..106474: state machine on obj+0x06 (1..6). */
static int step_state6(const f3mem_t *m, uint32_t obj)
{
    uint8_t s6 = rd8(m, obj + 0x06u);
    uint8_t s22 = rd8(m, obj + 0x22u);
    switch (s6) {
    case 1:
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
            int16_t x = (int16_t)rd16(m, obj + 0x14u);
            if (x >= -0x0A && x <= 0x011C)
                return 0;
        }
        wr8(m, obj + 0x22u, 0u);
        wr8(m, obj + 0x06u, 2u);
        return 0;
    case 2:
        switch (s22) {
        case 0:
            if ((rd8(m, obj + 0x4Eu) & 0x30u) == 0x30u) {
                wr8(m, obj + 0x22u, 1u);
                wr8(m, obj + 0x4Eu, (uint8_t)(rd8(m, obj + 0x4Eu) | 0x40u));
                Sound_PlayHooked(m, 0x0012u);
            }
            return 0;
        case 1:
            if ((rd8(m, obj + 0x4Eu) & 0x0Fu) == 0x0Fu && (int16_t)rd16(m, obj + 0x14u) <= 0x00FA)
                wr8(m, obj + 0x22u, 2u);
            return 0;
        case 2:
            if ((rd8(m, obj + 0x4Eu) & 0x0Fu) == 0x0Fu)
                (void)sub_1065B6(m, obj);
            return 0;
        default:
            return -1;
        }
    case 3:
    case 4:
        switch (s22) {
        case 0:
            if ((rd8(m, obj + 0x4Eu) & 0x0Fu) == 0x0Fu)
                wr8(m, obj + 0x22u, 1u);
            return 0;
        case 1: {
            uint8_t bit = ((rd8(m, obj + 0x4Eu) & 0x80u) != 0u) ? 5u : 4u;
            if ((rd8(m, obj + 0x4Eu) & (1u << bit)) != 0u) {
                wr8(m, obj + 0x4Eu, (uint8_t)(rd8(m, obj + 0x4Eu) ^ 0x80u));
                (void)sub_1065B6(m, obj);
                wr8(m, obj + 0x4Eu, (uint8_t)(rd8(m, obj + 0x4Eu) | 0x40u));
                Sound_PlayHooked(m, 0x001Bu);
            } else if ((rd8(m, obj + 0x4Eu) & 0x0Fu) == 0x0Fu) {
                wr8(m, obj + 0x06u, (s6 == 3u) ? 5u : 6u);
                wr8(m, obj + 0x22u, 0u);
            }
            return 0;
        }
        default:
            return -1;
        }
    case 5:
    case 6:
        switch (s22) {
        case 0:
        case 1:
        case 2:
        case 4:
            if ((rd8(m, obj + 0x4Eu) & 0x0Fu) == 0x0Fu) {
                uint8_t next = (uint8_t)(s22 + 1u);
                wr8(m, obj + 0x22u, next);
                if (next > 4u)
                    (void)sub_1065B6(m, obj);
            }
            return 0;
        case 3: {
            uint8_t bit = ((rd8(m, obj + 0x4Eu) & 0x80u) != 0u) ? 4u : 5u;
            if ((rd8(m, obj + 0x4Eu) & (1u << bit)) != 0u) {
                wr8(m, obj + 0x22u, 4u);
                wr8(m, obj + 0x4Eu, (uint8_t)(rd8(m, obj + 0x4Eu) | 0x40u));
                Sound_PlayHooked(m, 0x0012u);
            }
            return 0;
        }
        default:
            return -1;
        }
    default:
        return -1;
    }
}

/* Draw overlay sprite from table 0x10695A if +0x6A >= 0. */
static void draw_overlay_10695A(const f3mem_t *m, uint32_t obj)
{
    int16_t idx = (int16_t)rd16(m, obj + 0x6Au);
    if (idx < 0)
        return;

    uint32_t save_10 = rd32(m, obj + 0x10u);
    uint32_t save_14 = rd32(m, obj + 0x14u);
    uint32_t save_18 = rd32(m, obj + 0x18u);

    uint32_t rec = 0x0010695Au + (uint32_t)(int32_t)idx * 8u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, rec)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, rec + 2u)));
    wr32(m, obj + 0x10u, rd32(m, rec + 4u));
    Obj_DrawSprite(m, obj);

    wr32(m, obj + 0x18u, save_18);
    wr32(m, obj + 0x14u, save_14);
    wr32(m, obj + 0x10u, save_10);
}

/* Draw optional child sprite at +0x0C using +0x3A/+0x3E. */
static void draw_sub0c(const f3mem_t *m, uint32_t cur)
{
    uint32_t d0 = rd32(m, cur + 0x0Cu);
    if (d0 == 0u)
        return;

    uint32_t save_10 = rd32(m, cur + 0x10u);
    uint32_t save_14 = rd32(m, cur + 0x14u);
    uint32_t save_18 = rd32(m, cur + 0x18u);

    wr32(m, cur + 0x10u, d0);
    wr16(m, cur + 0x14u, rd16(m, cur + 0x3Au));
    wr16(m, cur + 0x18u, rd16(m, cur + 0x3Eu));
    Obj_DrawSprite(m, cur);

    wr32(m, cur + 0x18u, save_18);
    wr32(m, cur + 0x14u, save_14);
    wr32(m, cur + 0x10u, save_10);
}

/* ROM 0x10665A..106958: state machine on obj+0x21 (0..4). */
static int step_state21(const f3mem_t *m, uint32_t obj)
{
    switch (rd8(m, obj + 0x21u)) {
    case 0: {
        if (rd8(m, obj + 0x06u) <= 2u)
            return 0;
        sub_0D07B4(m, obj, 0x001069BAu);
        sub_0D0BBC(m, obj, 0x001069BAu);
        sub_0CFD66(m, obj, 0x00106A12u, rd16(m, obj + 0x08u));
        if (rd8(m, obj + 0x23u) != 0u) {
            sub_0D0384(m, obj, 0x00106B52u, rd16(m, obj + 0x08u));
            wr8(m, obj + 0x21u, 1u);
            return 0;
        }
        uint16_t save_42 = rd16(m, obj + 0x42u);
        sub_0D07B4(m, obj, 0x001069F2u);
        wr16(m, obj + 0x42u, save_42);
        if (rd8(m, obj + 0x23u) != 0u) {
            wr16(m, obj + 0x42u, 0x00B4u);
            wr8(m, obj + 0x21u, 2u);
        }
        return 0;
    }
    case 1:
        Obj_StepScript(m, obj);
        if (rd16(m, obj + 0x4Cu) == 0u) {
            sub_0CFD66(m, obj, 0x00106A12u, rd16(m, obj + 0x08u));
            wr8(m, obj + 0x21u, 4u);
            return 0;
        }
        {
            int16_t d1w = (int16_t)rd16(m, obj + 0x68u);
            if (d1w > 0) {
                uint32_t shot = Obj_Spawn(m, obj, 0x000D9224u, 9u, 2u, 0u);
                if ((int32_t)shot >= 0) {
                    wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 0x0028u));
                    wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x0028u));
                    wr16(m, shot + 0x58u, (uint16_t)d1w);
                    wr16(m, shot + 0x10u, 0x0028u);
                }
            } else if (d1w == 0) {
                uint32_t shot = Obj_Spawn(m, obj, 0x00108624u, 9u, 2u, 0u);
                if ((int32_t)shot >= 0) {
                    wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 0x0028u));
                    wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x0028u));
                    wr16(m, shot + 0x58u, rd16(m, obj + 0x08u));
                    wr16(m, shot + 0x10u, 0x0014u);
                }
            }
        }
        draw_overlay_10695A(m, obj);
        return 0;
    case 2:
        sub_0D0BBC(m, obj, 0x001069BAu);
        sub_0CFD66(m, obj, 0x00106A12u, rd16(m, obj + 0x08u));
        if (rd16(m, obj + 0x08u) == 0x00B4u) {
            Obj_StartBehaviour(m, obj, 0x0010700Au);
            wr8(m, obj + 0x21u, 3u);
        }
        return 0;
    case 3:
        Obj_StepScript(m, obj);
        if (rd16(m, obj + 0x4Cu) == 0u) {
            sub_0CFD66(m, obj, 0x00106A12u, rd16(m, obj + 0x08u));
            wr8(m, obj + 0x21u, 4u);
            return 0;
        }
        if ((int16_t)rd16(m, obj + 0x68u) >= 0) {
            uint32_t x = rd32(m, obj + 0x14u) + 0x00340000u;
            uint32_t y = rd32(m, obj + 0x18u) + 0x001F0000u;
            static const uint16_t angs[3] = { 0x0078u, 0x005Fu, 0x003Cu };
            for (int i = 0; i < 3; i++) {
                uint32_t shot = Obj_Spawn(m, obj, 0x000DBC1Au, 9u, 2u, 0u);
                if ((int32_t)shot < 0)
                    break;
                wr32(m, shot + 0x14u, x);
                wr32(m, shot + 0x18u, y);
                wr16(m, shot + 0x10u, 0x000Au);
                wr16(m, shot + 0x58u, angs[i]);
            }
        }
        draw_overlay_10695A(m, obj);
        return 0;
    case 4: {
        uint8_t cd = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, cd);
        if (cd == 0u) {
            wr8(m, obj + 0x21u, 0u);
            return 0;
        }
        uint8_t save_23 = cd;
        sub_0D07B4(m, obj, 0x001069BAu);
        wr8(m, obj + 0x23u, save_23);
        sub_0D0BBC(m, obj, 0x001069BAu);
        sub_0CFD66(m, obj, 0x00106A12u, rd16(m, obj + 0x08u));
        return 0;
    }
    default:
        return -1;
    }
}

/* ROM 0x105EDE: multi-part boss/enemy tick */
int Obj_105EDE_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd16(m, obj + 0x50u) != 0u)
        wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 1u));

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x1770u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x1770u);
        goto spawn_and_die;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto spawn_and_die;

    int16_t d0w = (int16_t)rd16(m, obj + 0x58u);
    int16_t d1w = (int16_t)rd16(m, obj + 0x5Cu);
    if ((rd8(m, obj + 0x4Eu) & 0x80u) != 0u) {
        d0w = (int16_t)rd16(m, obj + 0x60u);
        d1w = (int16_t)rd16(m, obj + 0x64u);
    }
    wr16(m, obj + 0x3Au, (uint16_t)(d0w - 0x27));
    wr16(m, obj + 0x3Eu, (uint16_t)(d1w - 0x2D));

    if ((int16_t)rd16(m, obj + 0x3Au) < -0x64 || (int16_t)rd16(m, obj + 0x3Eu) > 0x00F0) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    uint32_t a1 = rd32(m, obj + 0x0Cu);
    if ((rd8(m, obj + 0x4Eu) & 0x40u) != 0u) {
        wr8(m, obj + 0x4Eu, (uint8_t)(rd8(m, obj + 0x4Eu) & ~0x40u));
        wr16(m, a1 + 0x08u, 0x0168u);
    }
    if (rd16(m, a1 + 0x08u) != 0u) {
        uint32_t od0 = 0, od1 = 0;
        if (Obj_0CF898(m, obj, &od0, &od1) < 0)
            return -1;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + od1);
    }

    if (rd8(m, obj + 0x06u) != 1u && (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x06u, 1u);
            wr8(m, obj + 0x22u, 0u);
        }
    }

    if (step_state6(m, obj) < 0)
        return -1;
    if (step_state21(m, obj) < 0)
        return -1;

    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);

    uint32_t chain, dx, dy;
    if ((rd8(m, obj + 0x4Eu) & 0x80u) == 0u) {
        chain = rd32(m, obj + 0x54u);
        chain = rd32(m, chain + 0x54u);
        chain = rd32(m, chain + 0x54u);
        chain = rd32(m, chain + 0x54u);
        chain = rd32(m, chain + 0x54u);
        dx = rd32(m, obj + 0x58u) - rd32(m, obj + 0x60u);
        dy = rd32(m, obj + 0x5Cu) - rd32(m, obj + 0x64u);
    } else {
        chain = rd32(m, obj + 0x54u);
        dx = rd32(m, obj + 0x60u) - rd32(m, obj + 0x58u);
        dy = rd32(m, obj + 0x64u) - rd32(m, obj + 0x5Cu);
    }

    wr32(m, chain + 0x14u, rd32(m, chain + 0x14u) + dx);
    wr32(m, chain + 0x18u, rd32(m, chain + 0x18u) + dy);
    chain = rd32(m, chain + 0x54u);
    wr32(m, chain + 0x14u, rd32(m, chain + 0x14u) + dx);
    wr32(m, chain + 0x18u, rd32(m, chain + 0x18u) + dy);
    chain = rd32(m, chain + 0x54u);
    wr32(m, chain + 0x14u, rd32(m, chain + 0x14u) + dx);
    wr32(m, chain + 0x18u, rd32(m, chain + 0x18u) + dy);
    wr32(m, chain + 0x3Au, rd32(m, chain + 0x3Au) + dx);
    wr32(m, chain + 0x3Eu, rd32(m, chain + 0x3Eu) + dy);
    chain = rd32(m, chain + 0x54u);
    wr32(m, chain + 0x14u, rd32(m, chain + 0x14u) + dx);
    wr32(m, chain + 0x18u, rd32(m, chain + 0x18u) + dy);

    uint32_t cur = obj;
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    draw_sub0c(m, cur);
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    Obj_DrawSprite(m, rd32(m, cur + 0x08u));
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    draw_sub0c(m, cur);
    cur = rd32(m, cur + 0x54u); Obj_DrawSprite(m, cur);
    return 0;

spawn_and_die:
    {
        uint32_t fx = Obj_Spawn(m, obj, 0x000DC46Eu, 3u, 2u, 0u);
        if ((int32_t)fx >= 0) {
            wr32(m, fx + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, fx + 0x14u, (uint16_t)(rd16(m, fx + 0x14u) + 0x0020u));
            wr32(m, fx + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, fx + 0x18u, (uint16_t)(rd16(m, fx + 0x18u) + 0x0028u));
            wr16(m, fx + 0x58u, 0u);
            wr16(m, fx + 0x60u, 0u);
        }
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
}
