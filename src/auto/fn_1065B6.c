/* ROM 0x1065B6: register-level wrapper of sub_1065B6 (src/enemy/obj_105EDE.c)
 * for its capture.
 *   1065B6 clr.b $22(a0) ; state pick on $14(a0)/$50(a0)
 *   106606 / 106622 jsr $d0e3a (Obj_Next4Way) ; cmpi.w #imm,d0
 * D0 is the only register the body writes, and only on the two 0x0D0E3A
 * branches, where it holds that call's word (the capture's post D0 is
 * 0x00000000). Every other exit leaves the register file alone. */
#include "fn_auto.h"
#include "../enemy/obj_105EDE.h"

int Fn_1065B6(const f3mem_t *m, f3regs_t *r)
{
    int d0 = sub_1065B6(m, r->a[0]);            /* 0x1065B6..0x10661A */
    if (d0 >= 0)
        r->d[0] = (uint32_t)d0;                 /* 0x106606/0x106622 jsr 0x0d0e3a */
    return 0;                                   /* rts */
}
