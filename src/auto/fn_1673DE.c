/* ROM 0x1673DE: clears +0x44.l and arms 0x1673EA. */
#include "fn_auto.h"

int Fn_1673DE(const f3mem_t *m, f3regs_t *r)
{
    wr32(m, r->a[0] + 0x44u, 0u);                       /* 0x1673DE */
    wr32(m, r->a[0], 0x001673EAu);                      /* 0x1673E2 */
    return 0;
}
