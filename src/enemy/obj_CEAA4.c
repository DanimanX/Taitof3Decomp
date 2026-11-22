#include "obj_CEAA4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
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

/* ROM 0x0CEAA4: chain head wanders toward (+0x2C,+0x30); links follow */
int Obj_CEAA4_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd32(m, obj + 0x36u) == 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t a1 = rd32(m, obj + 0x36u);
    for (;;) {
        uint32_t a2 = rd32(m, a1 + 0x44u);
        if (a2 == 0u)
            break;
        uint8_t f = rd8(m, a2 + 0x22u);
        wr8(m, a1 + 0x22u, f);
        if (f == 0u) {
            wr32(m, a1 + 0x14u, rd32(m, a2 + 0x14u));
            wr32(m, a1 + 0x18u, rd32(m, a2 + 0x18u));
            wr16(m, a1 + 0x20u, rd16(m, a2 + 0x20u));
        }
        a1 = a2;
    }
    wr8(m, a1 + 0x22u, 0u);
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x2Cu) - rd16(m, obj + 0x14u));
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x30u) - rd16(m, obj + 0x18u));
    uint16_t ang = sub_006422(m, dx, dy);
    uint16_t d0 = (uint16_t)(Rng_NextMem(m) & 0x3Fu);
    d0 = (uint16_t)(uint8_t)((uint8_t)d0 - 0x20u + (uint8_t)ang);
    wr16(m, obj + 0x20u, d0);
    Obj_MoveByAngle(m, obj, d0);
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, a1 + 0x20u, rd16(m, obj + 0x20u));
    wr32(m, obj, 0xCEAA4u);
    return 0;
}
