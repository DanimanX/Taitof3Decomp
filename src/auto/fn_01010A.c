/* ROM 0x01010A: test-mode / boot idle entry ("   SERVICE SW ERROR   " screen,
 * reached by jmp 0x01010A from 0x000DF2 when the cabinet switches ask for test
 * mode). It never returns: the last instruction is `bra.s *` at 0x01014C.
 *
 * The capture therefore stops AT 0x01014C (F3_EXIT=01014C F3_INJECT_ABORT=1)
 * and this model runs the routine to exactly that point, register- and
 * RAM-accurate:
 *
 *   01010A  ori.w #0x700,SR        supervisor, mask 7 (SR is not in f3regs_t)
 *   01010E  moveq #0,D0 ; 010110 movec D0,CACR  (CACR is not in f3regs_t)
 *   010114  move.b #0,0x4A0004 / 01011C move.b #0,0x4A0014  write-only MMIO
 *   010124  move.l A0,-(A7)        save the string pointer
 *   010126  jsr 0x00117E           Sprite_ClearAll    (movem d0-d1/a0-a1)
 *   01012A  jsr 0x005A12           Gfx_ClearAllLayers (all five callees save
 *                                  d0-d2/a0-a1, so it is transparent)
 *   01012E  jsr 0x005A3A           character/font upload (movem d0-d1/a0-a1)
 *   010132  jsr 0x001C90           Video_UploadScroll: writes the playfield
 *                                  scroll registers and the sprite scroll
 *                                  words 0x600024/26 + 0x608024/26, and is
 *                                  the only callee here that does NOT restore
 *                                  the registers it uses (D0, D1, D2, A0)
 *   010136  movea.l (A7)+,A0       restore the string pointer
 *   010138  move.w #7,-(A7) ; 01013C move.l #0x61C792,-(A7) ; 010142 pea (A0)
 *   010144  jsr 0x005666           Text_DrawString: {col*2,char} pairs into
 *                                  text RAM 0x61C792, returns the end address
 *                                  in D0, restores D7/A0/A1
 *   010148  lea 0xA(A7),A7         drop the three arguments
 *   01014C  bra.s 0x01014C         endless spin
 *
 * At 0x01014C SP is the entry SP again (every push is popped, and each jsr's
 * rts pops its return address), A0/A1 and A2-A6 and D3-D7 are the caller's,
 * D1 = 0x0A and D2 = 0x09 from Video_UploadScroll's two moveq, and D0 is the
 * end of the drawn string. Verified on work/ram/fnt_01010A (ABORT capture:
 * entry 0x01010A -> 0x01014C, with F3_SPR=1 F3_GFX=1 so the sprite, layer,
 * character and text RAM writes are compared byte for byte). */
#include "fn_auto.h"
#include "fn_call_frames.h"
#include "../enemy/obj_1194C2.h"   /* Text_DrawString (ROM 0x005666) */
#include "../video/sprite.h"        /* Sprite_ClearAll 0x00117E, Gfx_ClearAllLayers
                                     * 0x005A12, Video_UploadScroll_Regs 0x001C90 */

int Fn_01010A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];

    /* 0x01010A ori.w #0x700,SR / 0x01010E moveq #0,D0 / 0x010110 movec D0,CACR:
     * SR and CACR are outside f3regs_t. 0x010114/0x01011C clear the write-only
     * control ports 0x4A0004 / 0x4A0014, not modelled (as in src/io/input.c). */
    push_long(m, r, a0);                            /* 0x010124 move.l A0,-(A7) */
    Sprite_ClearAll(m);                             /* 0x010126 jsr 0x00117E */
    Gfx_ClearAllLayers(m);                          /* 0x01012A jsr 0x005A12 */
    (void)Fn_005A3A(m, r);                          /* 0x01012E jsr 0x005A3A */
    Video_UploadScroll_Regs(m, r);                  /* 0x010132 jsr 0x001C90 */
    a0 = rd32(m, r->a[7]);                          /* 0x010136 movea.l (A7)+,A0 */
    r->a[7] += 4u;
    r->a[0] = a0;

    push_word(m, r, 7u);                            /* 0x010138 move.w #7,-(A7) */
    push_long(m, r, 0x0061C792u);                   /* 0x01013C move.l #0x61c792 */
    push_long(m, r, a0);                            /* 0x010142 pea.l (A0) */
    r->d[0] = Text_DrawString(m, a0, 0x0061C792u, 7u); /* 0x010144 jsr 0x005666 */
    r->a[7] += 0x0Au;                               /* 0x010148 lea 0xA(A7),A7 */
    return 0;                                       /* 0x01014C bra.s 0x01014C */
}
