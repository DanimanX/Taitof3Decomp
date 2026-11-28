/* ROM 0x00205C: TRAP #8. */
#include "../kernel/kernel.h"

int Fn_00205C(const f3mem_t *m, f3regs_t *r)
{
    Kern_00205C(m, r);
    Kern_RteFormat0(m, r);
    return 0;
}
