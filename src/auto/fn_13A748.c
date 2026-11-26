/* ROM 0x13A748: thunk branching directly to 0x13A6A2 (spinning boss debris/
 * enemy tick). */
#include "fn_auto.h"

int Fn_13A748(const f3mem_t *m, f3regs_t *r)
{
    return Fn_13A6A2(m, r);                                 /* 0x13A748 bra.w 0x13A6A2 */
}
