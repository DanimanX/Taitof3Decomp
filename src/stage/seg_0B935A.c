/* ROM 0x0B935A ending task 0x0B935A, in segments (src/kernel/seg.h).
 * Placeholder: not translated yet, every resume PC leaves translated code. */
#include "../kernel/seg.h"

uint32_t Seg_0B935A(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    (void)m; (void)r; (void)pc;
    return SEG_LEAVE;
}
