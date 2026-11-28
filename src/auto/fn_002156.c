/* ROM 0x002156: TRAP #10. */
#include "../kernel/kernel.h"

int Fn_002156(const f3mem_t *m, f3regs_t *r)
{
    Kern_002156(m, r);
    Kern_RteFormat0(m, r);
    return 0;
}
