/* ROM 0x0D5620: the spawn guard shared by the 0x0D5594/0x0D5620 family,
 * translated at register level for its capture (work/ram/fnk_0D5620).
 *
 *   0D5620  jsr 0x0CC1E0 ; tst.b D0 ; bne 0x0D5666   ; overlap list
 *   0D562C  jsr 0x0CC24A ; tst.b D0 ; bne 0x0D5666   ; on-screen box
 *   0D5638  move.w #0x10,D1 ; jsr 0x0CC306 ; tst.b D0 ; bne 0x0D5666
 *   0D5648  push arg 0x080D6000 / ngroups 2 / group 9 / fn 0x000D901A
 *   0D565C  jsr 0x01057C (Obj_Spawn) ; lea (0xC,A7),A7
 *   0D5666  rts
 *
 * The logic lives in Obj_CBF8A_0D5620 (src/enemy/obj_CBF8A.c); this wrapper
 * only carries the register file through it. The capture's three exit kinds:
 * the 0x0CC306 roll returning 0xFF (D0 = the RNG word | 0xFF), the roll
 * passing and the spawn leaving the slot address in D0, and D1 = (0x10 +
 * 0x0D0C9E word) >> 1 on both of the last two. */
#include "fn_auto.h"
#include "../enemy/obj_CBF8A.h"

int Fn_0D5620(const f3mem_t *m, f3regs_t *r)
{
    Obj_CBF8A_D5620Regs regs;
    regs.d0 = r->d[0];
    regs.d1 = r->d[1];
    regs.a1 = r->a[1];

    Obj_CBF8A_0D5620(m, r->a[0], &regs);

    r->d[0] = regs.d0;
    r->d[1] = regs.d1;
    r->a[1] = regs.a1;
    return 0;                                              /* 0x0D5666 rts */
}
