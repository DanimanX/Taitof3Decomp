#include "fn_auto.h"
/* ROM 0x091F0C: UNVERIFIED (weak capture): draw one NORMAL/CENT option label.
 * D1.W selects the text
 * (non-zero -> "CENT" at 0x091F51, zero -> "NORM" at 0x091F4C), both in
 * colour 6; D0 is the text-RAM cursor it is drawn at. D0 is saved and
 * restored, D1 and A0/A1 come back as they went in (0x005666 restores
 * D7/A0/A1).
 * The capture's RAM and gfx images are identical pre/post (D0 is restored and
 * text RAM was not re-dumped), so a do-nothing body also passes 30/30: the
 * strings are verified only against the bytes (0x091F4C "NORM", 0x091F51
 * "CENT"), and the capture never takes the CENT branch (D1.W = 0xFFFF in all
 * 30 pairs). */
#include "../kernel/fn_010352.h"   /* Kernel_005666 (0x005666) */

int Fn_091F0C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0];                                 /* 0x091F0C movem.l {D0},-(SP) */
    if ((uint16_t)r->d[1] != 0u)                           /* 0x091F10 tst.w D1w ; bne */
        Kernel_005666(m, 0x00091F51u, d0, 6u);             /* 0x091F30..0x091F42 "CENT" */
    else
        Kernel_005666(m, 0x00091F4Cu, d0, 6u);             /* 0x091F16..0x091F2C "NORM" */
    r->d[0] = d0;                                          /* 0x091F46 movem.l (SP)+,{D0} */
    return 0;                                              /* 0x091F4A rts */
}
