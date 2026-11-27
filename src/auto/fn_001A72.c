#include "fn_auto.h"
/* ROM 0x001A72: advance the random seed during kernel idle. */
#include "fn_call_frames.h"

int Fn_001A72(const f3mem_t *m, f3regs_t *r)
{
    push_long(m, r, 0x001A76);
    r->d[0] = Rng_NextMem(m);           /* 0x001D8A */
    r->a[7] += 8;                     /* callee and caller RTS */
    return 0;
}
