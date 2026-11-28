#include "fn_auto.h"
/* ROM 0x0C9752: release the object's palette and free it. A1 = the pointer at
 * A0+0x08 (its owner/definition record); the word at A1+0x22 indexes a ROM
 * long table at 0x0C92DA (PC-relative, 4-byte entries) giving the palette
 * source, and A1+0x0C (a colour byte) shifted left 6 and biased by 0x4000 is
 * the palette destination. 0x001660 queues that 0x20-entry upload
 * (A5-0x6750 slot list), then 0x0105EC frees the object. D0 ends as the table
 * long, D1 as the biased destination, A0/A1 as they went in. */
#include "../video/palette.h"        /* Pal_QueueUpload (0x001660) */
#include "../objects/obj_pool.h"     /* Obj_Free (0x0105EC) */

int Fn_0C9752(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1 = rd32(m, a0 + 0x08u);                    /* 0x0C9752 movea.l (8,A0),A1 */
    uint32_t d0 = rd16(m, a1 + 0x22u);                    /* 0x0C9756 move.w (0x22,A1),D0w */

    uint32_t d1 = rd8(m, a1 + 0x0Cu);                     /* 0x0C975A/5C move.b */
    d1 = (d1 << 6) & 0xFFFFu;                             /* 0x0C9760 lsl.w #6 */
    d1 = (d1 + 0x4000u) & 0xFFFFu;                        /* 0x0C9762 addi.w #0x4000 */

    /* 0x0C9766 move.l (0x0,PC,D0w*4),D0: base 0x0C9768 - 0x48E = 0x0C92DA,
     * word index sign-extended. */
    d0 = rd32(m, 0x000C92DAu + (uint32_t)((int32_t)(int16_t)(uint16_t)d0 * 4));

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;

    Pal_QueueUpload(m, d0, (uint16_t)d1, 0x20u);          /* 0x0C976C..0x0C9776 */
    Obj_Free(m, a0);                                      /* 0x0C9780..0x0C9782 */
    return 0;                                             /* 0x0C978C rts */
}
