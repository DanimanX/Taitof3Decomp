#include "obj_EF366.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0EF366: spawner, +0x21 children (0xEF400) every 0x26 frames */
int Obj_EF366_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;
    uint8_t c = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, c);
    if (c != 0u)
        return 0;
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x26u);
    uint32_t a1 = Obj_Spawn(m, obj, 0x000EF400u, 5u, 2u, 0u);
    if ((int32_t)a1 < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }
    uint32_t x = (uint32_t)(uint16_t)((Rng_Mix1D60(m) & 0x7Fu) + 0x91u);
    wr32(m, a1 + 0x14u, x << 16);
    uint32_t y = (uint32_t)(uint16_t)((Rng_Mix1D60(m) & 0x7Fu) + 0x2Cu);
    wr32(m, a1 + 0x18u, y << 16);
    wr8(m, a1 + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
