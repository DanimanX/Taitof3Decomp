/* ROM 0x003B96: clear the long word at A4 and return */
#include "ea_auto.h"

int Ea_003B96(const f3mem_t *m, f3regs_t *r)
{
    /* 0x003B96 clr.l (a4) */
    wr32(m, r->a[4], 0u);
    /* 0x003B98 rts */
    r->a[7] += 4u;
    return 0;
}
