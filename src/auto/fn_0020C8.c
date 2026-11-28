/* ROM 0x0020C8: TRAP #9. */
#include "../kernel/kernel.h"

int Fn_0020C8(const f3mem_t *m, f3regs_t *r)
{
    Kern_0020C8(m, r);
    Kern_RteFormat0(m, r);
    return 0;
}
