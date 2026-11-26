/* ROM 0x13BBE0: thunk bra.w 0x13BADE (A0 = object). */
#include "fn_auto.h"

int Fn_13BBE0(const f3mem_t *m, f3regs_t *r)
{
    return Fn_13BADE(m, r); /* 0x13BBE0 bra.w 0x13BADE */
}
