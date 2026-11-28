/* ROM 0x003FF0: clear the long word at A4 and return */
#include "ea_auto.h"

int Ea_003FF0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];

    wr32(m, r->a[4], 0u);                     /* 0x003FF0 clr.l (a4) */

    r->a[7] = sp + 4u;                        /* 0x003FF2 rts */
    return 0;
}
