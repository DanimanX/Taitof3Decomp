/* ROM 0x1385B2: draw compressed tilemap strip at tile (D0.w, D1.w) into
 * layer base A1, using D2[30:0] as ROM source and D2[31] as 0x02000000 attr
 * flag, via 0x0053B2. Preserves all registers. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_1385B2(const f3mem_t *m, f3regs_t *r)
{
    int16_t d0w = (int16_t)(uint16_t)r->d[0];
    int16_t d1w = (int16_t)(uint16_t)r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t flags = ((d2 >> 31) & 1u) ? 0x02000000u : 0u;   /* 0x1385B6..0x1385BE */
    int16_t off = (int16_t)((uint16_t)(d0w << 2) + (uint16_t)(d1w * 0x80)); /* 0x1385C0..0x1385C6 */
    uint32_t dst = (uint32_t)((int32_t)r->a[1] + (int32_t)off); /* 0x1385C8 */
    uint32_t src = d2 & 0x7FFFFFFFu;                         /* 0x1385CC */

    Gfx_DrawCompressedTilemap(m, src, dst, flags);           /* 0x1385D2 -> 0x0053B2 */
    return 0;
}
