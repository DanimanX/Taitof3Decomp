#include "fn_auto.h"

/* ROM 0x0EBEF0: checked against the ROM by Claude; no capture can exercise it (its writes never change memory): start the death sequence when the status byte is zero. */
#include "../objects/obj_common.h"

int Fn_0EBEF0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    if (rd8(m, obj + 0x22u) == 0) {
        wr8(m, obj + 6u, 0u); /* 0x0EBEFA */
        Obj_StartDying(m, obj); /* 0x0EBEFE tail branch to 0x0CEE74 */
    }
    r->a[7] += 4u;
    return 0;
}
