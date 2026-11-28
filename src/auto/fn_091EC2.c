#include "fn_auto.h"
/* ROM 0x091EC2: draw one ON/OFF option label (capture fnk_: F3_INJECT_REGS D0/D1, both arms).
 * D1.W selects the text
 * (non-zero -> "ON " at 0x091F02 with colour 8, zero -> "OFF" at 0x091F06
 * with colour 3); D0 is the text-RAM cursor it is drawn at. D0 is saved and
 * restored, D1 and A0/A1 come back as they went in (0x005666 restores
 * D7/A0/A1).
 * The capture's RAM and gfx images are identical pre/post (D0 is restored and
 * text RAM was not re-dumped), so a do-nothing body also passes 30/30: the
 * two strings themselves are verified only against the bytes (0x091F02 "ON ",
 * 0x091F06 "OFF"), and the capture never takes the OFF branch (D1.W = 0xFFFF
 * in all 30 pairs). */
#include "../kernel/fn_010352.h"   /* Kernel_005666 (0x005666) */

int Fn_091EC2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0];                                 /* 0x091EC2 movem.l {D0},-(SP) */
    if ((uint16_t)r->d[1] != 0u)                           /* 0x091EC6 tst.w D1w ; bne */
        Kernel_005666(m, 0x00091F02u, d0, 8u);             /* 0x091EE6..0x091EF8 "ON " */
    else
        Kernel_005666(m, 0x00091F06u, d0, 3u);             /* 0x091ECC..0x091EE2 "OFF" */
    r->d[0] = d0;                                          /* 0x091EFC movem.l (SP)+,{D0} */
    return 0;                                              /* 0x091F00 rts */
}
