/* ROM 0x112914: parallax foreground object initializer, record variant 1
 * (A0 = object). Resets the common per-frame fields via Obj_ResetFields
 * (0x0CBD86), copies the 4-long record at 0x112946 into +0x10/+0x14/+0x18/
 * +0x44, then re-arms the object with the tick at 0x1129BC. Entry stubs
 * 0x112914/0x11291E/0x112928/0x112932/0x11293C all `lea rec(PC),A1 ;
 * bra.w 0x112996` into the shared tail. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_112914(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x00112946u;                       /* 0x112914: lea 0x30(PC),A1 */

    Obj_ResetFields(m, obj);                         /* 0x11299A -> 0x0CBD86 */
    r->a[1] = a1 + 0x10u;                            /* 0x1129A4..0x1129B0: (A1)+ x4 */

    wr32(m, obj + 0x10u, rd32(m, a1 + 0x00u));       /* 0x1129A4: 0x0005052C */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x04u));       /* 0x1129A8: 0x01400000 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x08u));       /* 0x1129AC: 0x00700000 */
    wr32(m, obj + 0x44u, rd32(m, a1 + 0x0Cu));       /* 0x1129B0: 0x00000001 */

    wr32(m, obj, 0x001129BCu);                       /* 0x1129B4 */
    return 0;
}
