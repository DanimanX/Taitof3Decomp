#include "obj_109FAA.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x109FAA */
int Obj_109FAA_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x109FAA */
    if ((rd8(m, 0xF3EB5u) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    /* 0x109FB6 */
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    /* 0x109FC0 */
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;

    /* 0x109FC8 */
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t != 0u)
        return 0;

    /* 0x109FD0 */
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);

    /* 0x109FDA: pea 0 / move.w #2,-(SP) / move.w #5,-(SP) / move.l #0x10A024,-(SP) */
    uint32_t spawned = Obj_Spawn(m, obj, 0x0010A024u, 5u, 2u, 0u);
    /* 0x109FF6 */
    if ((int32_t)spawned < 0) {
        /* 0x10A01E */
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    /* 0x109FFC */
    uint32_t a1 = spawned;
    wr32(m, a1 + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x3Eu));
    wr8(m, a1 + 0x4Eu, 0u);

    /* 0x10A00E */
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));

    return 0;
}
