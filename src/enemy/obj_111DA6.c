#include "obj_111DA6.h"
#include "../objects/obj_common.h"

/* ROM 0x111DA6: reset fields, save +0x0C to +0x5C, load sprite/y/flip, arm 0x111DF0 */
int Obj_111DA6_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x5Cu, rd32(m, obj + 0x0Cu));
    uint32_t a1 = 0x00111DBCu;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, rd8(m, a1));
    wr32(m, obj + 0x00u, 0x00111DF0u);
    return 0;
}
