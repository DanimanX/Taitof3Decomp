/* ROM 0x13DAD4 stage end, in segments (src/kernel/seg.h).
 * Placeholder: not translated yet, every resume PC leaves translated code. */
#include "../kernel/seg.h"

uint32_t Seg_13DAD4(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    (void)m; (void)r; (void)pc;
    return SEG_LEAVE;
}
