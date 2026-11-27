/* ROM 0x11293C: object initializer from the 4-long record at 0x112986
 * (0x0005070C, 0x01400000, 0x008C0000, 0x00020000). Clears the per-frame
 * fields (0x0CBD86), copies the record to +0x10/+0x14/+0x18/+0x44, leaves A1
 * at the end of the record and arms 0x1129BC. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_11293C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x00112986u;                          /* 0x11293C lea (0x48,PC),A1 (PC=0x11293E) */

    Obj_ResetFields(m, obj);                            /* 0x11299A -> 0x0CBD86 */

    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;        /* 0x1129A4 */
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;        /* 0x1129A8 */
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;        /* 0x1129AC */
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;        /* 0x1129B0 */
    r->a[1] = a1;                                       /* 0x112996 push/pop A1 then four (A1)+ */

    wr32(m, obj, 0x001129BCu);                          /* 0x1129B4 */
    return 0;
}
