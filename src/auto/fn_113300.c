/* ROM 0x113300: thunk branching directly to 0x11320A (homing/orbiting sub-object tick). */
#include "fn_auto.h"

int Fn_113300(const f3mem_t *m, f3regs_t *r)
{
    return Fn_11320A(m, r);                              /* 0x113300 bra.w 0x11320A */
}
