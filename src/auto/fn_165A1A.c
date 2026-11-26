/* ROM 0x165A1A: line-RAM gradient fill tick thunk (branches to 0x1659F0). */
#include "fn_auto.h"

int Fn_165A1A(const f3mem_t *m, f3regs_t *r)
{
    return Fn_1659F0(m, r);                             /* 0x165A1A bra.b 0x1659F0 */
}
