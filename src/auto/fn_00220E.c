/* ROM 0x00220E: TRAP #12. */
#include "../kernel/kernel.h"

int Fn_00220E(const f3mem_t *m, f3regs_t *r)
{
    Kern_00220E(m, r);
    Kern_RteFormat0(m, r);
    return 0;
}
