#include "obj_116DD2.h"
#include "../objects/obj_common.h"

/* ROM 0x116DD2: link this object into the parent's (obj+8) +0x48 singly
 * linked list, after the existing head (the list is doubly linked through
 * +0x44/+0x48). */
int Obj_116DD2_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr16(m, obj + 0x28u, 0x0001u);
    wr16(m, obj + 0x2Cu, (uint16_t)-0x4);
    wr16(m, obj + 0x30u, (uint16_t)-0x4);
    wr16(m, obj + 0x2Eu, 0x0004u);
    wr16(m, obj + 0x32u, 0x0004u);
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t d0 = rd32(m, a1 + 0x48u);
    if (d0 == 0u) {
        wr32(m, a1 + 0x44u, obj);
        d0 = a1;
    }
    wr32(m, a1 + 0x48u, obj);
    uint32_t a2 = d0;
    wr32(m, a2 + 0x48u, obj);
    wr32(m, obj + 0x44u, a2);
    wr32(m, obj + 0x50u, 0u);
    wr32(m, obj + 0x00u, 0x116E22u);
    return 0;
}
