#include "fn_auto.h"
/* ROM 0x181960: continue the destruction animation or free the part when its parent is removed. */

int Fn_181960(const f3mem_t *m, f3regs_t *r)
{
    return Fn_181914(m, r);                             /* 0x181960 BRA.B */
}
