/* ROM 0x13B0B4: palette-animation object initialiser (A0 = object). Queues a
 * 6-word palette upload from 0x13B1E4 to palette entry 0x725C via 0x001660,
 * clears the two animation indices at +0x44/+0x48 and re-arms the object with
 * the tick 0x13B0E0. 0x001660 saves and restores D0-D2/A0/A2, so the register
 * file is unchanged at rts. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_13B0B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    /* 0x13B0B4 move.l #6,-(SP) ; 0x13B0BA move.l #0x725c,-(SP) ;
     * 0x13B0C0 move.l #0x13b1e4,-(SP) ; 0x13B0C6 jsr 0x001660 ;
     * 0x13B0CC lea (0xc,SP),SP */
    Pal_QueueUpload(m, 0x0013B1E4u, 0x725Cu, 6u);

    wr16(m, obj + 0x44u, 0u);                        /* 0x13B0D0 */
    wr16(m, obj + 0x48u, 0u);                        /* 0x13B0D4 */
    wr32(m, obj + 0x00u, 0x0013B0E0u);               /* 0x13B0D8 */
    return 0;
}
