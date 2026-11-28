/* ROM 0x01060E: register-level wrapper of Obj_ResetGroups (src/objects/obj_pool.h) for its capture.
 * Two word arguments on the stack: group.w at SP+4 (0x010612 `move.w $10(a7),d0`,
 * A7 = entry SP-12 after `movem.l d0-d1/a0,-(a7)`) and count.w at SP+6
 * (0x010616 `move.w $12(a7),d1`). The body fills one longword of 0xFFFFFFFF per
 * group from 0x410000+4*group (0x010620..0x010630) and is bracketed by the
 * movem pair at 0x01060E / 0x010632, so the register file is unchanged at rts. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_01060E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    Obj_ResetGroups(m, rd16(m, sp + 4u), rd16(m, sp + 6u));   /* 0x010612 / 0x010616 */
    return 0;
}
