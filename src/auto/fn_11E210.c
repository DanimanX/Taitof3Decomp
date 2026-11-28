/* ROM 0x11E210: re-arm entry of the 0x11E11C homing tick (A0 = object).
 * The ROM body is a single `bra.w 0x11E11C`, so the whole routine is the
 * 0x11E11C body: survive tests (HP +0x24, pause A5-0x144B bit 4, +0x35 == 2),
 * debug explosion (A5+0x1D58 bit 1 and X in [-0xA..0x11C] -> 0x0CBB2C), the
 * +0x06/+0x43 countdown, the +0x22 latch with aiming 0x0CC3C6 into +0x20 and
 * the difficulty-scaled +0x42 step 0x0CBEB0, then list push 0x0CBDA4, move
 * along +0x20 (0x0CBE12), colour, 0x0CBD52 animation, sprite draw (0x001282)
 * and either re-arm to 0x11E210 or replace the tick by the 0x11E252 free. */
#include "fn_auto.h"

int Fn_11E210(const f3mem_t *m, f3regs_t *r)
{
    /* 0x11E210 bra.w 0x11E11C: no stack frame, no register touch. */
    return Fn_11E11C(m, r);
}
