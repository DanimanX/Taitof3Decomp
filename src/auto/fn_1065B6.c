/* ROM 0x1065B6: register-level wrapper of sub_1065B6 (src/enemy/obj_105EDE.c)
 * for its capture.
 *   1065B6 clr.b $22(a0) ; 1065C2 move.w $14(a0),d0 ; state pick
 *   on $14(a0)/$50(a0)
 *   106606 / 106622 jsr $d0e3a (Obj_Next4Way) ; cmpi.w #imm,d0
 * D0 = word at +0x14 on the two rts paths that skip 0x0D0E3A (0x1065EE and
 * 0x1065F8), the 0x0D0E3A word on the two that take it (0x10661A, 0x106644),
 * and untouched when +0x50 == 0 (0x1065F6). */
#include "fn_auto.h"
#include "../enemy/obj_105EDE.h"

int Fn_1065B6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0];
    (void)sub_1065B6D0(m, r->a[0], &d0);        /* 0x1065B6..0x106644 */
    r->d[0] = d0;
    return 0;                                   /* rts */
}
