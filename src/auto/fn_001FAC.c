/* ROM 0x001FAC: TRAP #7. */
#include "../kernel/kernel.h"

int Fn_001FAC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t exit_pc = Kern_001FAC_Body(m, r);
    Kern_TrapReturn(m, r, exit_pc);
    return 0;
}
