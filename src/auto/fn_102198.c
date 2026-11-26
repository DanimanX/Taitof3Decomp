/* ROM 0x102198: attached child/spark initializer (A0 = object, +0x08 = parent).
 * Seeds combat/mover fields from 0x1086FC via 0x0CEDE4, installs behaviour
 * script 0x1021F4 via 0x0D0678, copies parent (+0x14, +0x18) with -8 on
 * +0x14.w, and arms 0x1021C8. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_102198(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x001086FCu);                             /* 0x102198..0x10219C */
    Obj_StartBehaviour(m, obj, 0x001021F4u);                     /* 0x1021A2..0x1021A6 */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | rd16(m, 0x001021F6u);    /* 0x0D0680 */

    uint32_t parent = rd32(m, obj + 0x08u);                      /* 0x1021AC */
    r->a[1] = parent;
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));               /* 0x1021B0 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u)); /* 0x1021B6 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));               /* 0x1021BA */
    wr32(m, obj, 0x001021C8u);                                   /* 0x1021C0 */
    return 0;
}
