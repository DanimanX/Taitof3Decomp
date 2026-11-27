#ifndef TAITOF3_KERNEL_KERNEL_H
#define TAITOF3_KERNEL_KERNEL_H
/* Task kernel: scheduler (0x002274) and TRAP #1..#12 system calls.
 *
 * Each routine runs from its entry to the RTE that ends it (the RTE itself is
 * not executed: SR/PC are outside f3regs_t, so A7 is left pointing at the
 * exception frame) and returns the PC of that RTE. A call that ends in the
 * scheduler (bra 0x002274) returns Kern_Schedule's result: 0x0022DC when a
 * task context was restored, 0x002310 for the idle context. Verified with
 * test_fn SEGREGS entries (D0-D7, A0-A6, SP, the RTE reached, RAM). */
#include "../common/f3mem.h"
#include "../auto/fn_auto.h"

uint32_t Kern_Schedule(const f3mem_t *m, f3regs_t *r); /* ROM 0x002274 */

/* TRAP #1..#12, each from its entry to the RTE (or bra 0x002274) that ends it.
 * The jsr gates in front of the handlers (0x1E32, 0x1E60, 0x1F1A, 0x1F62, ...)
 * that store SR are not translated: f3regs_t does not hold SR. */
uint32_t Kern_001E76(const f3mem_t *m, f3regs_t *r); /* ROM 0x001E76 TRAP #1  task create */
uint32_t Kern_001E0E(const f3mem_t *m, f3regs_t *r); /* ROM 0x001E0E TRAP #2  task wake */
uint32_t Kern_001E48(const f3mem_t *m, f3regs_t *r); /* ROM 0x001E48 TRAP #3  task exit */
uint32_t Kern_001F30(const f3mem_t *m, f3regs_t *r); /* ROM 0x001F30 TRAP #4  task sleep */
uint32_t Kern_001F78(const f3mem_t *m, f3regs_t *r); /* ROM 0x001F78 TRAP #5  task yield */
uint32_t Kern_001FE8(const f3mem_t *m, f3regs_t *r); /* ROM 0x001FE8 TRAP #6  task event wait */
uint32_t Kern_001FAC(const f3mem_t *m, f3regs_t *r); /* ROM 0x001FAC TRAP #7  task sleep, no stack fix */
uint32_t Kern_00205C(const f3mem_t *m, f3regs_t *r); /* ROM 0x00205C TRAP #8  wake mask */
uint32_t Kern_0020C8(const f3mem_t *m, f3regs_t *r); /* ROM 0x0020C8 TRAP #9  task start/stop */
uint32_t Kern_002156(const f3mem_t *m, f3regs_t *r); /* ROM 0x002156 TRAP #10 task release */
uint32_t Kern_0021BE(const f3mem_t *m, f3regs_t *r); /* ROM 0x0021BE TRAP #11 task hand over */
uint32_t Kern_00220E(const f3mem_t *m, f3regs_t *r); /* ROM 0x00220E TRAP #12 task word or */

#endif
