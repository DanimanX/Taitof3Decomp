#include "obj_F238A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0F238A */
int Obj_F238A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0F238A: hit-stop / timeout -> die */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0F23A0 */
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;                                        /* 0x0F2410 */

    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    if (rd8(m, obj + 0x23u) != 0u)
        return 0;
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);

    /* 0x0F23BA: Obj_Spawn(parent=obj, fn=0x0F241C, group 5, ngroups 2, arg 0) */
    uint32_t o = Obj_Spawn(m, obj, 0x000F241Cu, 5u, 2u, 0u);
    if ((int32_t)o < 0) {
        /* 0x0F2412 */
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    wr32(m, o + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, o + 0x18u, rd32(m, obj + 0x3Eu));
    wr32(m, o + 0x10u, rd32(m, obj + 0x0Cu));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 0x28u));
    uint16_t d0w = rd16(m, obj + 0x54u);
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + d0w));
    wr8(m, o + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, o + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
