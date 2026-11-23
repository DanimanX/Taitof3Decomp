#include "obj_114A76.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */
static uint16_t sub_006422(const f3mem_t *m, uint16_t d0, uint16_t d1)
{
    uint32_t a0 = 0x026E40u;
    uint16_t d2 = 0u;

    if ((int16_t)d0 < 0) {
        d2 |= 0x0002u;
        d0 = (uint16_t)(0u - d0);
    }
    if ((int16_t)d1 < 0) {
        d2 |= 0x0004u;
        d1 = (uint16_t)(0u - d1);
    }

    uint16_t src = ((int16_t)d1 > (int16_t)d0) ? d1 : d0;
    uint16_t d3 = 0x20u;
    for (uint16_t i = 0u; i < 16u; i++) {
        if (src & (uint16_t)(0x8000u >> i)) {
            d3 = (uint16_t)(0x10u + i);
            break;
        }
    }

    d3 = (uint16_t)(d3 - 0x1Au);
    if ((int16_t)d3 < 0) {
        d3 = (uint16_t)(0u - d3);
        unsigned cnt = (unsigned)d3 & 63u;
        d0 = (cnt >= 16u) ? 0u : (uint16_t)(d0 >> cnt);
        d1 = (cnt >= 16u) ? 0u : (uint16_t)(d1 >> cnt);
    }

    if (d2 != 0u && d2 != 6u) {
        uint16_t tmp = d0;
        d0 = d1;
        d1 = tmp;
    }

    d1 = (uint16_t)(d1 << 6);
    a0 += (uint32_t)(int32_t)(int16_t)d1;
    uint8_t tbl = rd8(m, a0 + (uint32_t)(int32_t)(int16_t)d0);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint16_t)tbl);
    a0 = 0x006492u;
    d0 = (uint16_t)(d0 + rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d2));
    d0 &= 0x00FFu;
    return d0;
}

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x1136BC: register as a hazard for player 1 (+0x22 == 0, A5+0x906)
 * or player 2 (A5+0x902), then move along angle +0x20 */
static void sub_1136BC(const f3mem_t *m, uint32_t obj)
{
    uint32_t list = rd16(m, obj + 0x22u) == 0u ? F3_A5 + 0x0906u : F3_A5 + 0x0902u;
    uint32_t a1 = rd32(m, list);
    wr32(m, a1, obj);
    wr32(m, list, a1 + 4u);
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
}

/* cmp2.w (abs).l,Dn; bcs -> out of the signed bounds at addr */
static int out_of(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* ROM 0x114A76: homing missile */
int Obj_114A76_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u)
        goto die;
    if (rd8(m, rd32(m, rd32(m, obj + 0x08u) + 0x08u) + 0x50u) & 0x10u)
        goto die;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto die;
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x14u) + 0x10u),
                                   (int16_t)(rd16(m, obj + 0x18u) + 0x10u)) != 0u)
        goto die;
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    sub_1136BC(m, obj);
    if ((int16_t)rd16(m, obj + 0x42u) < 0x14 && (rd16(m, F3_A5 + 0x0878u) & 1u) == 0u) {
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
    } else {
        /* 0x114AF8: steer towards the target at +0x3A */
        uint16_t rate = (uint16_t)(rd16(m, obj + 0x06u) + 0x70u);
        wr16(m, obj + 0x06u, rate);
        if ((int16_t)rate <= 0x0700) {
            uint32_t a1 = rd32(m, obj + 0x3Au);
            uint16_t dx = (uint16_t)(rd16(m, a1 + 0x14u) - rd16(m, obj + 0x14u));
            uint16_t dy = (uint16_t)(rd16(m, a1 + 0x18u) - rd16(m, obj + 0x18u));
            int8_t d = (int8_t)(uint8_t)((uint8_t)sub_006422(m, dx, dy) - rd8(m, obj + 0x20u));
            if (d > 0)
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + rate));
            else if (d < 0)
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - rate));
        }
    }

    /* 0x114B40 */
    wr8(m, obj + 0x1Cu, (uint8_t)((rd8(m, obj + 0x1Cu) + 1u) & 0x03u));
    Obj_DrawSprite(m, obj);
    if (out_of(m, 0x000CC112u, rd16(m, obj + 0x14u)) || out_of(m, 0x000CC116u, rd16(m, obj + 0x18u)))
        goto die;
    wr32(m, obj + 0x00u, 0x00114A76u);
    return 0;

die:
    Obj_Free(m, obj);
    return 0;
}
