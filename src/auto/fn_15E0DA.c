/* ROM 0x15E0DA: stage palette build. Sixteen times it blends the 64-byte
 * master palette at 0x23FC0 with the palette at 0x16A290 at weight i*0x800
 * (0/0x800/../0x7800, i.e. a cross-fade over the sixteen steps) into the
 * Work-RAM palette buffers 0x40B3AE + i*0x40 (0x16A286 -> 0x16A258), then
 * queues each buffer for palette RAM at 0x3C00 + i*0x40 with 0x10 words
 * (0x1660). D0-D3/A0-A2 are saved around every step; only D7 is left at
 * 0xFFFF by the blend loop. */
#include "fn_auto.h"
#include "../video/palette.h"

int Fn_15E0DA(const f3mem_t *m, f3regs_t *r)
{
    for (unsigned i = 0; i < 16u; i++) {
        uint32_t dst = 0x0040B3AEu + i * 0x40u;         /* 0x15E0E4 + i*0x40 */
        uint16_t weight = (uint16_t)(i * 0x0800u);      /* 0x15E0EA move.w #(i*0x800),D0w */
        Pal_Blend64(m, dst, 0x00023FC0u, 0x0016A290u, weight); /* 0x15E0F2 jsr 0x16a286 */
        Pal_QueueUpload(m, dst, (uint16_t)(0x3C00u + i * 0x40u), 0x10u); /* 0x15E106 jsr 0x1660 */
    }
    /* 0x15E47A rts: D7.w = 0xFFFF (0x16A258's dbf), everything else restored. */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | 0xFFFFu;
    return 0;
}
