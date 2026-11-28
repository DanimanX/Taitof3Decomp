/* ROM 0x010672: register-level wrapper of Obj_ResetGroup11 (src/objects/obj_pool.h) for its capture.
 * No arguments: group 11 only (0x010676 `move.w #$b,d0`, 0x01067A `move.w #$1,d1`)
 * — one longword of 0xFFFFFFFF at 0x41002C and at 0x41005C (0x010694/0x01069A).
 * The body is bracketed by `movem.l d0-d1/a0-a1,-(a7)` / `movem.l (a7)+`
 * (0x010672 / 0x0106A6), so every register except SP is unchanged at rts. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_010672(const f3mem_t *m, f3regs_t *r)
{
    (void)r;
    Obj_ResetGroup11(m);
    return 0;
}
