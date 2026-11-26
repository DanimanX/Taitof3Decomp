/* ROM 0x10E734: child/effect object init (A0 = object).
 * Seeds combat params from 0x10C8E2 via 0x0CEDE4, installs behaviour record
 * 0x10E79A via 0x0D0678, sets palette override +0x1D = 0x5C, copies position
 * from parent (+0x08) with X offset -4, and arms 0x10E76A. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

int Fn_10E734(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x0010C8E2u);                                           /* 0x10E734..0x10E738 */
    Obj_StartBehaviour(m, obj, 0x0010E79Au);                                   /* 0x10E73E..0x10E742 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x0010E79Cu);                  /* 0x0D0680 */

    wr8(m, obj + 0x1Du, 0x5Cu);                                                /* 0x10E748 */
    uint32_t parent = rd32(m, obj + 0x08u);                                    /* 0x10E74E */
    r->a[1] = parent;
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));                             /* 0x10E752 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 4u));               /* 0x10E758 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));                             /* 0x10E75C */
    wr32(m, obj, 0x0010E76Au);                                                 /* 0x10E762 */
    return 0;
}
