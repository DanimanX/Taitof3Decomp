/* ROM 0x0D609A: register-level wrapper of OBJ_0D609A (src/enemy/obj_D609A.c) for its capture. */
/* Body: 0D609A move.l A0,-(SP) ; jsr 0x105EC (free self) ; lea 4(SP),SP ; rts.
 * 0x105EC saves D0/A0-A1 and restores them, and the A0 push is popped by the
 * `lea 4(SP),SP` before the rts, so no register changes at the return (the
 * capture's only delta is SP, which the harness accounts for on its own).
 * A0 = object pointer. */
#include "fn_auto.h"
#include "../enemy/obj_D609A.h"

int Fn_0D609A(const f3mem_t *m, f3regs_t *r)
{
    (void)Obj_D609A_Tick(m, r->a[0]);   /* 0x0D609C */
    return 0;                           /* 0x0D60A6 rts */
}
