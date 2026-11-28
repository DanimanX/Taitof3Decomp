/* ROM 0x13F2B6: (capture fnt_: up to the first yield): stage-timeline task restart entry.
 * It re-runs the stage-start wrapper 0x13F428 (stage setup + terrain + banner,
 * clears A5-0x144C bit 5), installs the ROM script base 0x0040C2B4 as the
 * wave-script pointer A5+0x3308, and falls into the task's TRAP #5 yield at
 * 0x13F2CA (0x13F2C2 bra.w 0x13F2CA), so only the pre-yield code belongs to
 * this entry. The per-frame loop is TaskStageTimeline_Frame (Fn_13F2C6.c).
 * 0x13F2C6 is a separate entry that repeats the wrapper without the store. */
#include "fn_auto.h"

int Fn_13F2B6(const f3mem_t *m, f3regs_t *r)
{
    if (Fn_13F428(m, r) < 0)                             /* 0x13F2B6 bsr.w 0x13F428 */
        return -1;
    wr32(m, F3_A5 + 0x3308u, 0x0040C2B4u);               /* 0x13F2BA move.l #0x40c2b4,(0x3308,A5) */
    return 0;                                            /* 0x13F2C2 bra.w 0x13F2CA: trap #5 */
}
