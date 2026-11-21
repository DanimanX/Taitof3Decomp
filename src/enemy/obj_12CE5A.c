#include "obj_12CE5A.h"
#include "../objects/obj_pool.h"

/* ROM 0x12CE5A */
int Obj_12CE5A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12CE5A */
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, d0);
    if ((int16_t)d0 > 0) {
        wr32(m, obj, 0x0012CE5Au);          /* 0x12CE52 */
        return 0;
    }

    /* 0x12CE60: spawn 0x12E19A (groups 3..4) */
    uint32_t child = Obj_Spawn(m, obj, 0x0012E19Au, 3u, 2u, 0u);
    if (child == 0xFFFFFFFFu)
        return 0;

    /* 0x12CF44 */
    wr32(m, F3_A5 + 0x30DEu,
         (child < 0x408000u ? child + 0x400000u : child));
    wr16(m, F3_A5 + 0x32A0u, 0u);
    wr8(m, F3_A5 + 0x32AEu, 0x00u);
    wr8(m, F3_A5 + 0x329Cu, 0x00u);
    wr8(m, F3_A5 + 0x329Du, 0x00u);
    wr16(m, F3_A5 + 0x329Eu, 0u);
    Obj_Spawn(m, obj, 0x0012EA24u, 3u, 2u, 0u);
    Obj_Spawn(m, obj, 0x0012F26Au, 3u, 2u, 0u);
    Obj_Spawn(m, obj, 0x0011C1DEu, 4u, 1u, 0u);
    Obj_Spawn(m, obj, 0x0011C39Au, 4u, 1u, 0u);

    /* 0x12CFCC */
    wr32(m, obj, 0x0012CFD4u);
    return 0;
}
