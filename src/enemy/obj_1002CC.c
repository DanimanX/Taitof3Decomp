#include "obj_1002CC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
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
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}
/* ROM 0x0D4A2C: D2w selects the 4-byte entry 0x10000 - 0x0202*D2w at
 * 0x0D4A5A (the high word is x, the low word y). D1w and D0w index the
 * 0x0D4E5A factor table, applied to x and y respectively; the results come
 * back as D0 (x * k1) and D1 (x * k0, swapped). */
static uint32_t sub_0D4E5A_scale(uint32_t v, uint16_t sel)
{
    switch (sel & 0x3Fu) {
    case 0: case 52: return v;                    /* 0D4FF4 / 0D4FF8 */
    case 1: return v + v;                         /* 0D4FF8 */
    case 2: case 53: return v + v;                /* 0D4FFC |
                                                    0D4FFA: add.l D1,D1 */
    case 3: return v * 3u;                        /* 0D5006 */
    case 4: case 55: return v << 2;               /* 0D5010 / 0D500E */
    case 5: case 56: return v * 5u;               /* 0D501A / 0D5014 */
    case 6: case 57: return v * 6u;               /* 0D502A / 0D5022 */
    case 7: case 58: return v * 7u;               /* 0D503A / 0D5034 */
    case 8: case 59: return v << 3;               /* 0D5044 / 0D5042 */
    case 9: case 60: return v * 9u;               /* 0D504E / 0D5048 */
    case 10: case 61: return v * 10u;             /* 0D505E / 0D5056 */
    case 11: case 62: return v * 11u;             /* 0D5072 / 0D5068 */
    case 12: case 63: return v * 13u;             /* 0D5086 / 0D507E */
    case 13: return v * 14u;                      /* 0D509A */
    case 14: return v * 15u;                      /* 0D50AE */
    case 15: return v * 16u;                      /* 0D50BE */
    case 16: return v * 17u;                      /* 0D50C8 */
    case 17: return v * 18u;                      /* 0D50D2 */
    case 18: return v * 19u;                      /* 0D50E2 */
    case 19: return v * 20u;                      /* 0D50F6 */
    case 20: return v * 21u;                      /* 0D510A */
    case 21: return v * 22u;                      /* 0D511E */
    case 22: return v * 23u;                      /* 0D5136 */
    case 23: return v * 24u;                      /* 0D514E */
    case 24: return v * 25u;                      /* 0D5162 */
    case 25: return v * 26u;                      /* 0D5176 */
    case 26: return v * 27u;                      /* 0D518E */
    case 27: return v * 28u;                      /* 0D51A6 */
    case 28: return v * 29u;                      /* 0D51BA */
    case 29: return v * 30u;                      /* 0D51CE */
    case 30: return v * 31u;                      /* 0D51E2 */
    case 31: return v * 32u;                      /* 0D51F2 */
    case 32: return v * 33u;                      /* 0D51FC */
    case 33: return v * 34u;                      /* 0D5206 */
    case 34: return v * 35u;                      /* 0D5216 */
    case 35: return v * 36u;                      /* 0D522A */
    case 36: return v * 37u;                      /* 0D523E */
    case 37: return v * 38u;                      /* 0D5252 */
    case 38: return v * 39u;                      /* 0D526A */
    case 39: return v * 41u;                      /* 0D5284 */
    case 40: return v * 42u;                      /* 0D529A */
    case 41: return v * 43u;                      /* 0D52AE */
    case 42: return v * 44u;                      /* 0D52C6 */
    case 43: return v * 45u;                      /* 0D52E2 */
    case 44: return v * 46u;                      /* 0D52FE */
    case 45: return v * 47u;                      /* 0D531A */
    case 46: return v * 48u;                      /* 0D5338 */
    case 47: return v * 49u;                      /* 0D5354 */
    case 48: return v * 50u;                      /* 0D536A */
    case 49: return v * 51u;                      /* 0D537E */
    case 50: return v * 52u;                      /* 0D5396 */
    case 51: return 0u;                           /* 0D4FF2: clr.l D1 */
    case 54: return v * 3u;                       /* 0D5000 */
    default: return v;
    }
}

static void sub_0D4A2C(const f3mem_t *m, uint16_t d0w, uint16_t d1w, uint16_t d2w,
                       uint16_t *out_d0, uint16_t *out_d1)
{
    uint32_t xy = rd32(m, 0x000D4A5Au + (uint32_t)d2w * 4u);
    uint32_t r1 = sub_0D4E5A_scale(xy, d1w);
    uint32_t r0 = sub_0D4E5A_scale(xy, d0w);
    *out_d0 = (uint16_t)r1;
    *out_d1 = (uint16_t)r0;
}
/* ROM 0x1002CC */
int Obj_1002CC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);      /* parent */

    /* 0x1002D0 */
    if (rd8(m, a4 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x1002DA */
    wr32(m, obj + 0x14u, rd32(m, a4 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x22u));
    wr16(m, obj + 0x18u, 0x00ACu);

    /* 0x1002EC */
    sub_0D0690(m, obj);

    /* 0x1002F2 */
    int16_t d68 = (int16_t)rd16(m, obj + 0x68u);
    if (d68 >= 0) {
        if (d68 == 0) {
            /* 0x1002FE: move.w #0x24,-(SP); jsr 0x13F238; jsr Sound_PlayCmd */
            Sound_PlayCmd(m, 0x0024);
        }
        /* 0x100312 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x14u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
        Obj_DrawSprite(m, obj);
        return 0;
    }

    /* 0x100324 */
    uint16_t d2 = (uint16_t)(rd16(m, a4 + 0x18u) + 0x0Au);
    d2 = (uint16_t)(d2 - rd16(m, obj + 0x18u));
    if ((int16_t)d2 < 0)
        d2 = (uint16_t)(0u - d2);
    wr8(m, obj + 0x1Eu, (uint8_t)d2);
    wr8(m, obj + 0x1Fu, (uint8_t)d2);

    /* 0x10033E: D0w = 0x18, D1w = 0x1C, D2w = obj+0x68 */
    uint16_t d0, d1;
    sub_0D4A2C(m, 0x0018u, 0x001Cu, rd16(m, obj + 0x68u), &d0, &d1);
    /* 0x10034C: D0w = D0w + D0w, D0w = -D0w, D0w += 0x30 */
    d0 = (uint16_t)(0u - (uint16_t)(d0 + d0));
    d0 = (uint16_t)(d0 + 0x30u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    d1 = (uint16_t)(0u - d1);
    d1 = (uint16_t)(d1 + 0x1Cu);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d1));

    /* 0x100362 */
    Obj_DrawSprite(m, obj);
    return 0;
}
