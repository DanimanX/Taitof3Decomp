/* ROM 0x16373C: sets +0x60.w = 0xFF and arms 0x16374A. */
#include "fn_auto.h"

int Fn_16373C(const f3mem_t *m, f3regs_t *r)
{
    wr16(m, r->a[0] + 0x60u, 0x00FFu);                  /* 0x16373C */
    wr32(m, r->a[0], 0x0016374Au);                      /* 0x163742 */
    return 0;
}
