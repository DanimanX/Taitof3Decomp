#include "obj_F7BD2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x0F7BD2 */
int Obj_F7BD2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0F7BD2: die on hit-stop or when the parent has expired */
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if ((uint8_t)(rd8(m, obj + 0x20u) | rd8(m, obj + 0x22u)) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x0F7BEC */
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;                                        /* 0x0F7C3C */

    wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
    if (rd8(m, obj + 0x23u) != 0u)
        return 0;
    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x28u);

    /* 0x0F7C06: Obj_Spawn(parent=obj, fn=0x0F7C48, group 5, ngroups 2, arg 0) */
    uint32_t o = Obj_Spawn(m, obj, 0x000F7C48u, 5u, 2u, 0u);
    if ((int32_t)o < 0) {
        /* 0x0F7C3E */
        wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) - 1u));
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    wr8(m, o + 0x4Eu, 0u);
    if (rd8(m, obj + 0x21u) == 0u)
        wr8(m, o + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
