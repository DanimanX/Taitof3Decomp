/* ROM 0x001FE8: TRAP #6. */
#include "../kernel/kernel.h"

int Fn_001FE8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t exit_pc = Kern_001FE8_Body(m, r);
    if (exit_pc == 0x002274u)
        r->sr &= (uint16_t)~0x0010u; /* ADD.W D1,D1 clears X for task IDs 0-15. */
    Kern_TrapReturn(m, r, exit_pc);
    return 0;
}
