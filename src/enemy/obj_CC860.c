#include "obj_CC860.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

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
        d0 = (uint16_t)(d0 >> (d3 & 63u));
        d1 = (uint16_t)(d1 >> (d3 & 63u));
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

/* ROM 0x0CC33A: angle from (obj - (d0,d1)) to the centre of the target at
 * A5+0x1D28 (0x80 when none) */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0, uint16_t d1)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
    if (a1 == 0u)
        return 0x0080u;
    d0 = (uint16_t)(0u - d0);
    d0 = (uint16_t)(d0 + (uint16_t)((int16_t)(rd16(m, a1 + 0x2Cu) + rd16(m, a1 + 0x2Eu)) >> 1));
    d0 = (uint16_t)(d0 + rd16(m, a1 + 0x14u) - rd16(m, obj + 0x14u));
    d1 = (uint16_t)(0u - d1);
    d1 = (uint16_t)(d1 + (uint16_t)((int16_t)(rd16(m, a1 + 0x30u) + rd16(m, a1 + 0x32u)) >> 1));
    d1 = (uint16_t)(d1 + rd16(m, a1 + 0x18u) - rd16(m, obj + 0x18u));
    return sub_006422(m, d0, d1);
}

/* ROM 0x0CC860: spinning shot; when the timer ends aim at the target */
int Obj_CC860_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 8u));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 8u));
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    Obj_DrawSprite(m, obj);
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000CC860u);
        return 0;
    }
    uint16_t d0 = sub_0CC33A(m, obj, 8u, 8u);
    wr16(m, obj + 0x20u, d0);
    /* 0x0CC79E */
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + 4u));
    d0 = (uint16_t)(d0 >> 3);
    wr32(m, obj + 0x10u, rd32(m, 0x000CC8C4u + (uint32_t)(int32_t)(int16_t)d0 * 4u));
    wr16(m, obj + 0x06u, 0x0026u);
    wr32(m, obj + 0x00u, 0x000CC7C0u);
    return 0;
}
