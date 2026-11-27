/* ROM 0x14FD34: build and upload the 16-entry palette ramp for a fade step.
 * For i = 0..15 it blends a 64-byte palette (0x150F3C for even i, 0x150F7C
 * for odd i) with 0x150DBC into the Work RAM buffer 0x40B3AE + i*0x40 using
 * the weight (i/2)*0x780 (0x16A258), then queues that buffer as a 16-word
 * upload to palette RAM 0x3000 + i*0x40 (0x001660). D0-D3 and A0-A2 are
 * saved/restored around every iteration; only D7 is left changed (0xFFFF). */
#include "fn_auto.h"
#include "../video/palette.h"      /* Pal_Blend64 (0x16A258), Pal_QueueUpload (0x001660) */

int Fn_14FD34(const f3mem_t *m, f3regs_t *r)
{
    for (uint32_t i = 0; i < 16u; i++) {
        uint32_t a0 = 0x0040B3AEu + i * 0x40u;             /* 0x14FD44 + i*0x40 */
        uint32_t src1 = (i & 1u) ? 0x00150F7Cu : 0x00150F3Cu;
        uint16_t w = (uint16_t)((i / 2u) * 0x780u);        /* 0x14FD4A, 0x78 increments */
        Pal_Blend64(m, a0, src1, 0x00150DBCu, w);          /* 0x14FD54 jsr 0x16a258 */
        Pal_QueueUpload(m, a0, (uint16_t)(0x3000u + i * 0x40u), 0x0010u); /* 0x14FD68 jsr 0x1660 */
    }
    r->d[7] = 0x0000FFFFu;                                 /* 0x16A280 dbf leaves D7.w = -1 */
    return 0;                                              /* 0x150154 rts */
}
