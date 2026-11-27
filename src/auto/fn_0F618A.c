#include "fn_auto.h"
/* ROM 0x0F618A: checked against the ROM by Claude; no capture can exercise it (its writes never change memory): start the dying state when the object byte at +0x22 is zero. */
#include "../objects/obj_common.h"

int Fn_0F618A(const f3mem_t *m, f3regs_t *r)
{
    if (rd8(m, r->a[0] + 0x22u) == 0u)
        Obj_StartDying(m, r->a[0]); /* 0x0F618E tail branch to 0x0CEE74 */
    r->a[7] += 4u;
    return 0;
}
