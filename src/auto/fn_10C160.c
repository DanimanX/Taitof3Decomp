/* ROM 0x10C160: seed enemy stats via 0x0CEDE4 from 0x10C222, clear +0x34 bit 1, arm 0x10C178. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_10C160(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x0010C222u);                     /* 0x10C160..0x10C164 */
    r->a[1] = 0x0010C23Eu;                               /* 0x0CEDE4 leaves A1 at +0x1C */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x10C16A */
    wr32(m, obj, 0x0010C178u);                           /* 0x10C170 */
    return 0;
}
