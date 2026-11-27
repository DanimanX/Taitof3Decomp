#include "fn_auto.h"
#include "../kernel/kernel.h"
/* ROM 0x002274: scheduler loop; see Kern_Schedule (src/kernel/kernel.c). */

int Fn_002274(const f3mem_t *m, f3regs_t *r)
{
    (void)Kern_Schedule(m, r);
    return 0;
}
