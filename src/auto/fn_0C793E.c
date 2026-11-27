#include "fn_auto.h"

/* ROM 0x0C793E: UNVERIFIED (weak capture): install the next object state. */
int Fn_0C793E(const f3mem_t *m, f3regs_t *r)
{
    wr32(m, r->a[0], 0x0C7946u);
    r->a[7] += 4u;
    return 0;
}
