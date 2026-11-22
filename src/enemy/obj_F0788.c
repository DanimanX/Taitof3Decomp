#include "obj_F0788.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0D0E3A: 4-way interleaved global counter in A5+0x1D60 */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0F0788 */
int Obj_F0788_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u || rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    if (rd8(m, obj + 0x21u) == 0u)
        return 0;

    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t != 0u)
        return 0;

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x19u);

    uint32_t child = Obj_Spawn(m, obj, 0x000F0814u, 5u, 2u, 0u);
    if ((int32_t)child < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
    uint16_t dy = (uint16_t)((sub_0D0E3A(m) & 0x3Fu) - 0x20u);
    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + dy));
    wr8(m, child + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, child + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
