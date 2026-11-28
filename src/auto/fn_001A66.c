/* ROM 0x001A66: UNVERIFIED (no capture): level-1 interrupt entry thunk.
 * It is one instruction, `jmp 0x00222E` (bytes 4E F9 00 00 22 2E), i.e. an
 * unconditional tail jump into the VBlank preempt at 0x00222E, which is
 * already translated as Kern_00222E (src/kernel/kernel.c: save the interrupted
 * context, run one frame tick, rescan the task table, RTE). Its body cannot be
 * executed here because the frame tick polls the 0x4A0000 input ports and
 * f3regs_t carries no port image; with an assumed all-released image the
 * coin/service branch of 0x000DFA (the 0x01010A hardware halt) is taken
 * instead of a plain tick. Entered with no image, Kern_00222E leaves the
 * register file untouched and reports 0. The preempt never returns to this
 * instruction, and the thunk has no register, stack or RAM effect of its own. */
#include <stddef.h>
#include "fn_auto.h"
#include "../kernel/kernel.h"

int Fn_001A66(const f3mem_t *m, f3regs_t *r)
{
    uint32_t tick_sp = 0;

    (void)Kern_00222E(m, r, NULL, &tick_sp);      /* 0x001A66 jmp 0x00222E */
    return 0;
}
