/* ROM 0x14477C: register-level wrapper of sub_14477C (src/auto/fn_144378.c)
 * for its capture.
 * A0 walks 8-byte rows from the cursor at A5+0x3320 until the +6 word of the
 * row is negative, and the rts leaves that row pointer in A0. The helper
 * returns exactly the value the ROM's last `lea 8(a0),a0` / entry value holds,
 * and touches no other register. */
#include "fn_auto.h"
#include "fn_helpers.h"

int Fn_14477C(const f3mem_t *m, f3regs_t *r)
{
    r->a[0] = sub_14477C(m);                       /* 0x14477C..0x1447AA */
    return 0;
}
