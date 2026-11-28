/* ROM 0x001A66: UNVERIFIABLE (no capture can show it): level-1 interrupt entry
 * thunk -- the IRQ3 handler's second arm.
 *
 *   001A54  tst.b A5-0x5D4E ; 001A58 beq 0x1A66
 *   001A5C  st.b A5-0x5D4D ; 001A60 sf.b A5-0x5D4E ; 001A64 rte
 *   001A66  jmp 0x00222E
 *
 * 0x001A66 is exactly one instruction, `jmp 0x00222E` (bytes 4E F9 00 00 22 2E):
 * nothing observable happens before the jump. It writes no register, no stack
 * byte and no RAM, so an ABORT capture could only stop at its target 0x00222E,
 * where the state is bit-for-bit the pre state -- a pair that an empty Fn
 * passes too, i.e. no evidence about this address. The target is verified on
 * its own captures (tests/test_fn.c "Kernel 0x00222E preempt Z01/Z04",
 * work/ram/fn_00222E and fn_00222E_z04), and what the tail jump does there is
 * Kern_00222E's business, not this thunk's.
 *
 * The thunk is therefore modelled as the tail jump into Kern_00222E
 * (src/kernel/kernel.c: save the interrupted context, run one frame tick,
 * rescan the task table, RTE). Its body cannot be executed in the harness
 * anyway because the frame tick polls the 0x4A0000 input ports and f3regs_t
 * carries no port image; with an assumed all-released image the coin/service
 * branch of 0x000DFA (the 0x01010A hardware halt) is taken instead of a plain
 * tick. Entered with no image, Kern_00222E leaves the register file untouched
 * and reports 0. */
#include <stddef.h>
#include "fn_auto.h"
#include "../kernel/kernel.h"

int Fn_001A66(const f3mem_t *m, f3regs_t *r)
{
    uint32_t tick_sp = 0;

    (void)Kern_00222E(m, r, NULL, &tick_sp);      /* 0x001A66 jmp 0x00222E */
    return 0;
}
