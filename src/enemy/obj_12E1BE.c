#include "obj_12E1BE.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x12E1BE */
int Obj_12E1BE_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12E1BE: hold, then set the scroll velocity and chain onwards */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        /* 0x12E1B6 */
        wr32(m, obj, 0x0012E1BEu);
        return 0;
    }

    /* 0x12E1C4: A1 = (*(A5+0x32AA))+0x48 */
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    wr32(m, a1 + 0x56u, 0x00500000u);
    wr32(m, a1 + 0x5Au, 0x00260000u);
    wr32(m, a1 + 0x5Eu, 0u);
    wr16(m, F3_A5 + 0x32A2u, 0x4000u);
    wr16(m, obj + 0x06u, 0x0002u);

    /* 0x12E1F0: beq -- the subq above left Z clear, so this is 0x12E1F2 */
    wr32(m, obj, 0x0012E1FAu);
    return 0;
    return 0;
}
