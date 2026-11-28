/* ROM 0x0021BE: TRAP #11. */
#include "../kernel/kernel.h"

int Fn_0021BE(const f3mem_t *m, f3regs_t *r)
{
    Kern_0021BE(m, r);
    Kern_RteFormat0(m, r);
    return 0;
}
