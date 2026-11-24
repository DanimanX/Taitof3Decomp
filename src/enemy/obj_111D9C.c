#include "obj_111D9C.h"
#include "../objects/obj_common.h"

/* ROM 0x111D9C: init a child from the spawn argument {sprdef.l, x.l, y.l, flip.b}
 * and chain on to 0x111DF0. The trailing nop + bra.w share the 0x111DC8
 * prologue with 0x111DA6. */
int Obj_111D9C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a1 = 0x111DB0u;                       /* lea (0x12,PC),A1 */
    wr32(m, obj + 0x5Cu, rd32(m, obj + 0x0Cu));
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, rd8(m, a1));
    wr32(m, obj + 0x00u, 0x111DF0u);
    return 0;
}
