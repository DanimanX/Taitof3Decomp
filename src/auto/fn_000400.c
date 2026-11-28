/* ROM 0x000400: spurious-interrupt vector, a single RTE. */
#include "../kernel/kernel.h"

int Fn_000400(const f3mem_t *m, f3regs_t *r)
{
    Kern_RteFormat0(m, r);
    return 0;
}
