/* ROM 0x12B7C2: reset an object's common fields via Obj_ResetFields
 * (0x0CBD86), clear the byte at +0x34 (sf) and re-arm the tick 0x12B7D4. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_12B7C2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_ResetFields(m, obj);                         /* 0x12B7C2 -> 0x0CBD86 */
    wr8(m, obj + 0x34u, 0x00u);                      /* 0x12B7C8: sf (0x34,A0) */
    wr32(m, obj, 0x0012B7D4u);                       /* 0x12B7CC */
    return 0;
}
