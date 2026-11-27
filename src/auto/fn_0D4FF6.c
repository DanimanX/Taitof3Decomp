#include "fn_auto.h"
/* ROM 0x0D4FF6: UNVERIFIED (weak capture): return immediately. */

int Fn_0D4FF6(const f3mem_t *m, f3regs_t *r)
{
    (void)m;
    r->a[7] += 4u; /* 0x0D4FF6 RTS */
    return 0;
}
