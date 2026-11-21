#include "obj_ECA04.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0D0E3A: 16-bit LCG at A5+0x1D60 (n = n*5+1), returns n >> 8 */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0ECA04: "popcorn" emitter; drops a 0x0ECAC8 projectile below itself
 * every 0x1E frames while obj+0x21 > 0. */
int Obj_ECA04_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 - 0x144Bu) & 0x20u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;                                /* 0x0ECAC0 rts */

    /* 0x0ECA22 */
    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    if (rd8(m, obj + 0x23u) != 0u)
        return 0;

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x1Eu);
    uint32_t a1 = Obj_Spawn(m, obj, 0x000ECAC8u, 5, 2, 0u);
    if ((int32_t)a1 < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    /* 0x0ECA56 */
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    uint16_t d0 = (uint16_t)((sub_0D0E3A(m) & 0x3Fu) - 0x20u);
    wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + d0));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    d0 = (uint16_t)((sub_0D0E3A(m) & 0x1Fu) - 0x10u);
    wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + d0));

    wr32(m, a1 + 0x60u, 0x00210000u);
    d0 = (uint16_t)(sub_0D0E3A(m) & 0x07u);
    wr16(m, a1 + 0x60u, (uint16_t)(rd16(m, a1 + 0x60u) + d0));
    wr32(m, a1 + 0x64u, 0x000101D0u);
    wr32(m, a1 + 0x50u, 0x00019C40u);
    wr8(m, a1 + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
