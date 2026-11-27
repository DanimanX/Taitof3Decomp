/* ROM 0x110660: re-arms the object state pointer: (A0) = 0x110668. */
#include "fn_auto.h"

int Fn_110660(const f3mem_t *m, f3regs_t *r)
{
    wr32(m, r->a[0], 0x00110668u);                      /* 0x110660 */
    return 0;
}
