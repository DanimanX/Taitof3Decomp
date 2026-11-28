/* ROM 0x001F62 Sys_Yield: (abort capture fnt_, recipes17cw2.tsv): the
 * "yield to the scheduler" call gate. It builds the 6-byte TRAP #5 frame for
 * the current PC and falls through into the TRAP #5 body at 0x001F78, so the
 * routine never reaches an rts of its own and a capture of it cannot close on
 * a return.
 *
 *   001F62  move.w SR,(A5-0x1458)          ; settings base 0x406BA8
 *   001F66  ori.w #0x700,SR
 *   001F6A  move.l (SP),-(2,SP)            ; PC copy, overlapping store
 *   001F6E  clr.w (2,SP)
 *   001F72  subq.l #2,SP
 *   001F74  move.w (A5-0x1458),-(SP)       ; SR word below the saved PC
 *   001F78  ... TRAP #5 handler body (0x001F78, kernel.c Kern_001F78)
 *
 * Capture stop: 0x001F78, the first instruction of the TRAP #5 body, with
 * F3_INJECT=001F62 F3_EXIT=001F78 F3_INJECT_ABORT=1
 * (work/zcap/recipes17cw2.tsv). At that PC SP is the entry SP - 4 and the frame
 * is [SR word at SP, the caller's return PC at SP+2..SP+5 with the word at SP+4
 * cleared] -- those bytes lie below the entry SP, so test_fn does not compare
 * them, but SP is compared, and so is the word at 0x406BA8.
 *
 * Fn_001F62 runs exactly the code before that first TRAP body and returns 0
 * (the frame at SP is what 0x001F78 expects). The word at A5-0x1458 is the
 * caller's SR: the only direct evidence is work/ram/fn_001F62, where all 30
 * pairs leave 0x200C at 0x406BA8 (S=1, interrupt mask 0, CC bits 0x0C from
 * the instructions before the jsr). The CC half is caller-dependent and is not
 * modelled anywhere in this tree, so this routine stores the observed 0x200C;
 * if the ABORT capture above shows another value at 0x406BA8, the model needs
 * the caller's SR word as an input. */
#include "fn_auto.h"

int Fn_001F62(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint16_t sr = r->sr ? r->sr : 0x200Cu;          /* caller SR (F3_SRCOL captures) */
    uint32_t pc;

    wr16(m, a5 - 0x1458u, sr);                     /* 0x001F62 move.w SR */
    pc = rd32(m, r->a[7]);                         /* 0x001F6A move.l (SP),-(2,SP) */
    wr32(m, r->a[7] - 2u, pc);
    wr16(m, r->a[7] + 2u, 0u);                     /* 0x001F6E clr.w (2,SP) */
    r->a[7] -= 2u;                                 /* 0x001F72 subq.l #2,SP */
    r->a[7] -= 2u;                                 /* 0x001F74 move.w ...,-(SP) */
    wr16(m, r->a[7], sr);
    return 0;                                      /* 0x001F78 the TRAP #5 body */
}
