#ifndef TAITOF3_KERNEL_SEG_H
#define TAITOF3_KERNEL_SEG_H
/* Task code translated in segments.
 *
 * Code that yields in the middle (TRAP #5 frame yield, TRAP #4 sleep) cannot
 * be a C function that runs to its rts: the kernel switches tasks at every
 * yield. Such code is translated as segments: a segment starts at a resume
 * PC (the instruction after a TRAP, or a routine entry reached by jsr/rts
 * inside a segment) and runs to the next TRAP #4/#5, which it returns.
 *
 * Segments are register-accurate (the 68k registers in r carry from one
 * segment to the next, a[7] = SP) and keep the 68k stack in memory for the
 * return addresses they cross: a jsr pushes its return PC (seg_push32) and
 * the callee's segment continues; an rts pops it and Seg_Run dispatches the
 * caller's continuation. Stack bytes below SP (saved registers, pushed
 * arguments) are only written where a later segment reads them.
 *
 * Verified with scripts/seg_capture.sh <resume> <yields> (one capture per
 * resume PC, the exit PC in the regs.tsv PC column) and test_fn's SEGREGS
 * entries, which compare RAM, D0-D7, A0-A6, SP and the TRAP reached. */
#include "../common/f3mem.h"
#include "../auto/fn_auto.h"

/* segment left translated code (unknown resume PC or a path not translated) */
#define SEG_LEAVE 0xFFFFFFFFu

/* run one segment from pc; returns the PC of the TRAP #4/#5 reached */
uint32_t Seg_Run(const f3mem_t *m, f3regs_t *r, uint32_t pc);

/* one group per routine whose code yields; each returns as Seg_Run */
uint32_t Seg_13F5B8(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* stage outro */
uint32_t Seg_13DAD4(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* stage end */
uint32_t Seg_13D69A(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* high-score name entry */
uint32_t Seg_0B935A(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* ending task 0x0B935A */
uint32_t Seg_0B8404(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* attract / title sequencer (Task_Attract) */
uint32_t Seg_19F504(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode MONITOR TEST */
uint32_t Seg_19F708(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode screen (yields via jsr 0x1F62) */
uint32_t Seg_1A00E6(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode SOUND TEST (0x1A00E6) */
uint32_t Seg_1A043C(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode SOUND TEST (0x1A043C) */
uint32_t Seg_1A0942(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode SOUND TEST wrapper (0x1A0942) */
uint32_t Seg_1A095E(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode HI-SCORE RESET (0x1A095E) */
uint32_t Seg_1A0A52(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode CONFIGURATION */
uint32_t Seg_1A1282(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode DIFFICULTY */
uint32_t Seg_1A1856(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode FACTORY SETTING */
uint32_t Seg_1A1A26(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode main menu (Task_TestModeMenu) */
uint32_t Seg_1A1944(const f3mem_t *m, f3regs_t *r, uint32_t pc);   /* test-mode EXIT */

static inline void seg_push32(const f3mem_t *m, f3regs_t *r, uint32_t v)
{
    r->a[7] -= 4u;
    wr32(m, r->a[7], v);
}

static inline uint32_t seg_pop32(const f3mem_t *m, f3regs_t *r)
{
    uint32_t v = rd32(m, r->a[7]);
    r->a[7] += 4u;
    return v;
}

/* set the low word of a data register (move.w/addq.w/... leave the high word) */
static inline void seg_setw(uint32_t *d, uint16_t v) { *d = (*d & 0xFFFF0000u) | v; }

#endif
