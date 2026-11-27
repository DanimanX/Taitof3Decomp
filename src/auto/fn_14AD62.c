/* ROM 0x14AD62: build and upload the 16-stage palette ramp. For i = 0..15
 * it blends the 64-byte palette 0x234C0 with the 64-byte weighting table
 * 0x16A290 into the Work RAM buffer 0x40B7AE + i*0x40 with weight
 * 0x5A00 - i*0x600 (0x16A286 -> 0x16A258) and queues that buffer as a
 * 16-word upload to palette RAM 0x2000 + i*0x40 (0x001660). D0-D3, A0-A2 and
 * the rest are saved/restored around every iteration, so only D7 changes: the
 * dbf counter of 0x16A258 leaves it at 0xFFFF. */
#include "fn_auto.h"
#include "../video/palette.h"      /* Pal_Blend64 (0x16A258), Pal_QueueUpload (0x001660) */

int Fn_14AD62(const f3mem_t *m, f3regs_t *r)
{
    for (uint32_t i = 0; i < 16u; i++) {
        uint32_t a0 = 0x0040B7AEu + i * 0x40u;             /* 0x14AD6C + i*0x40 */
        uint16_t w = (uint16_t)(0x5A00u - i * 0x600u);     /* 0x14AD72 + i*-0x600 */
        Pal_Blend64(m, a0, 0x000234C0u, 0x0016A290u, w);   /* 0x14AD7A jsr 0x16a286 -> 0x16a258 */
        Pal_QueueUpload(m, a0, (uint16_t)(0x2000u + i * 0x40u), 0x0010u); /* 0x14AD8E jsr 0x1660 */
    }
    r->d[7] = 0x0000FFFFu;                                 /* 0x16A280 dbf leaves D7.w = -1 */
    return 0;                                              /* 0x14B102 rts */
}
