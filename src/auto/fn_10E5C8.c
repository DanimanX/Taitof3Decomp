/* ROM 0x10E5C8: spawner init from parameter block 0x10C8E2 (A0 = object).
 * Calls Obj_0CEDE4 (0x0CEDE4) with A1 = 0x10C8E2 (advances A1 to 0x10C8FE),
 * sets +0x23.b = 1, and arms 0x10E5E0. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_10E5C8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Obj_0CEDE4(m, obj, 0x0010C8E2u);                     /* 0x10E5C8..0x10E5CC */
    r->a[1] = 0x0010C8FEu;
    wr8(m, obj + 0x23u, 1u);                             /* 0x10E5D2 */
    wr32(m, obj, 0x0010E5E0u);                           /* 0x10E5D8 */
    return 0;
}
