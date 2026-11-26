/* ROM 0x10E662: attached sub-object initializer (A0 = object, +0x08 = parent).
 * Seeds combat/mover fields from 0x10C8E2 via 0x0CEDE4, installs behaviour
 * script 0x10E6C8 via 0x0D0678, sets +0x1D = 0x5C, copies parent (+0x14, +0x18)
 * with +4 to high word of +0x14, and transitions to 0x10E698. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_10E662(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x0010C8E2u);                         /* 0x10E662..0x10E666 */
    Obj_StartBehaviour(m, obj, 0x0010E6C8u);                 /* 0x10E66C..0x10E670 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x0010E6CAu); /* 0x0D0680 */

    wr8(m, obj + 0x1Du, 0x5Cu);                              /* 0x10E676 */
    uint32_t parent = rd32(m, obj + 0x08u);                  /* 0x10E67C */
    r->a[1] = parent;
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));           /* 0x10E680 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 4u)); /* 0x10E686 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));           /* 0x10E68A */
    wr32(m, obj, 0x0010E698u);                               /* 0x10E690 */
    return 0;
}
