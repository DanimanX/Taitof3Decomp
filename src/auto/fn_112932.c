/* ROM 0x112932: parallax foreground object initializer, record variant 4
 * (A0 = object). Same shared tail as 0x112914 (0x112996): Obj_ResetFields
 * (0x0CBD86), copy the 4-long record at 0x112976 into +0x10/+0x14/+0x18/
 * +0x44, then re-arm with the tick at 0x1129BC. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_112932(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x00112976u;                       /* 0x112932: lea 0x42(PC),A1 */

    Obj_ResetFields(m, obj);                         /* 0x11299A -> 0x0CBD86 */
    r->a[1] = a1 + 0x10u;                            /* 0x1129A4..0x1129B0: (A1)+ x4 */

    wr32(m, obj + 0x10u, rd32(m, a1 + 0x00u));       /* 0x1129A4: 0x00050688 */
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x04u));       /* 0x1129A8: 0x01400000 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x08u));       /* 0x1129AC: 0x00780000 */
    wr32(m, obj + 0x44u, rd32(m, a1 + 0x0Cu));       /* 0x1129B0: 0x0001CCCC */

    wr32(m, obj, 0x001129BCu);                       /* 0x1129B4 */
    return 0;
}
