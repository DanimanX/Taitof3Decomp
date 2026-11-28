/* ROM 0x18BB50: register-level wrapper of Obj_18BB50 (src/enemy/obj_18B9DE.c),
 * the score-credit + 0x179DEE burst routine reached from the 0x18BAAE and
 * 0x18BB12 vtable entries.
 *
 *   18BB50  jsr 0x17B02E ; tst.l D0 ; blt ; movea.l D0,A1 ; addi.l #0x7530,(0x48,A1)
 *   18BB66  addi.l #0x12C5F,(0x886,A5) ; bfset #2,(0x4B26,A5)
 *   18BB74  move.w (0x34,A0),(0x4B20,A5) ; move.l (0x36,A0),(0x4B1C,A5)
 *   18BB80  bset.b #2,(0x23,A0) ; tst.b (0x22,A0) -> spawn group 0/1 or 5/2
 *   18BBAC  move.w #0x13,-(A7) ... (sound 0x13)
 *
 * Only D0 is written on the captured path: the 0x01057C spawn leaves the slot
 * address there and 0x0039D8 (Sound_PlayCmd) preserves D0. A1 takes the
 * 0x17B02E credit pointer on the objects whose +0x35 == 2 (fnk_18BB50 pairs
 * 2/5/...: A1 = 0x41E340); pairs whose +0x35 != 2 keep the caller's A1. */
#include "fn_auto.h"
#include "../enemy/obj_18B9DE.h"

int Fn_18BB50(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a1 = r->a[1];
    r->d[0] = Obj_18BB50R(m, r->a[0], &a1);  /* 0x18BB50..0x18BBB2, D0 = slot */
    r->a[1] = a1;                            /* 0x18BB5C movea.l D0,A1 (credit) */
    return 0;                                /* rts */
}
