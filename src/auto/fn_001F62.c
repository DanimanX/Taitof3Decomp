/* ROM 0x001F62 Sys_Yield: UNVERIFIED (no capture): the "yield to the
 * scheduler" call gate. It builds the 6-byte TRAP #5 frame for the current PC
 * and falls through into the TRAP #5 body at 0x001F78, so the routine never
 * reaches an rts of its own; every captured pair ends a frame later and
 * tests/test_fn.c skips all 30 (VBlank-interrupted).
 *
 *   001F62  move.w SR,(A5-0x1458)          ; settings base 0x406BA8
 *   001F66  ori.w #0x700,SR
 *   001F6A  move.l (SP),-(2,SP)            ; PC copy, overlapping store
 *   001F6E  clr.w (2,SP)
 *   001F72  subq.l #2,SP
 *   001F74  move.w (A5-0x1458),-(SP)       ; SR word below the saved PC
 *   001F78  ... TRAP #5 handler body (0x001F78, kernel.c Kern_001F78)
 *
 * Fn_001F62 runs exactly the code before that first TRAP body and returns 0
 * (the frame at SP is what 0x001F78 expects). The word at A5-0x1458 is the
 * caller's SR: the only direct evidence is work/ram/fn_001F62, where all 30
 * pairs leave 0x200C at 0x406BA8 (S=1, interrupt mask 0, CC bits 0x0C from
 * the instructions before the jsr). The CC half is caller-dependent and is
 * not modelled anywhere in this tree, so this routine stores the observed
 * 0x200C. */
#include "fn_auto.h"

int Fn_001F62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t sr = 0x200Cu;
    uint32_t pc;

    wr16(m, a5 - 0x1458u, sr);                     /* 0x001F62 move.w SR */
    pc = rd32(m, r->a[7]);                         /* 0x001F6A move.l (SP),-(2,SP) */
    wr32(m, r->a[7] - 2u, pc);
    wr16(m, r->a[7] + 2u, 0u);                     /* 0x001F6E clr.w (2,SP) */
    r->a[7] -= 2u;                                 /* 0x001F72 subq.l #2,SP */
    r->a[7] -= 2u;                                 /* 0x001F74 move.w ...,-(SP) */
    wr16(m, r->a[7], sr);
    return 0;                                      /* the TRAP #5 body follows */
}
