/* ROM 0x19F708 test-mode screen (entered from Task_TestModeMenu 0x1A1A26), in
 * segments (src/kernel/seg.h). It yields with jsr 0x001F62 (Sys_Yield call
 * gate) at 0x19F742 instead of a TRAP: a segment stops AT that jsr and
 * returns 0x19F742; the next one resumes at 0x19F748. The screen ends at the
 * RTS at 0x19FFEE (return through seg_pop32 as usual).
 * Placeholder: not translated yet, every resume PC leaves translated code. */
#include "../kernel/seg.h"

uint32_t Seg_19F708(const f3mem_t *m, f3regs_t *r, uint32_t pc)
{
    (void)m; (void)r; (void)pc;
    return SEG_LEAVE;
}
