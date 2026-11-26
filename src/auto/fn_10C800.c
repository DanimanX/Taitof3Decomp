/* ROM 0x10C800: enemy init from parameter block 0x10C8C2 (A0 = object).
 * Calls Obj_0CEDE4 (0x0CEDE4) with A1 = 0x10C8C2 (advances A1 to 0x10C8DE),
 * clears bit 1 of +0x34.b, and arms 0x10C818. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_10C800(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    Obj_0CEDE4(m, obj, 0x0010C8C2u);                     /* 0x10C800..0x10C804 */
    r->a[1] = 0x0010C8DEu;
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x10C80A */
    wr32(m, obj, 0x0010C818u);                           /* 0x10C810 */
    return 0;
}
