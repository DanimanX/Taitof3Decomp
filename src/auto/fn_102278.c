/* ROM 0x102278: init parent-attached emitter (A0 = object, +0x08 = parent).
 * Calls 0x0CEDE4 with table 0x1086FC, sets +0x21 = 0x8C, copies parent +0x18.l
 * to +0x3E.l, adds 0x70 to +0x3E.w, leaves A1 = parent, and arms 0x1022A0. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_102278(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Obj_0CEDE4(m, obj, 0x001086FCu);                         /* 0x102278..0x10227C */
    wr8(m, obj + 0x21u, 0x8Cu);                              /* 0x102282 */
    uint32_t a1 = rd32(m, obj + 0x08u);                      /* 0x102288 */
    r->a[1] = a1;
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x18u));               /* 0x10228C */
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 0x0070u)); /* 0x102292 */
    wr32(m, obj, 0x001022A0u);                               /* 0x102298 */
    return 0;
}
