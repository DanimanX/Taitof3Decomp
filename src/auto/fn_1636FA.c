/* ROM 0x1636FA: compressed tilemap swap tick (A0 = object).
 * Decrements +0x06; while > 0 re-arms 0x1636FA. On expiry draws compressed
 * tilemap 0x6B5FA to 0x611000 via 0x0053B2, sets +0x06 = 2, and transitions
 * to 0x163732 (the subsequent beq is never taken since 2 != 0). */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_1636FA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);     /* 0x1636FA */
    wr16(m, obj + 0x06u, (uint16_t)cnt);

    if (cnt > 0) {                                          /* 0x1636FE */
        wr32(m, obj, 0x001636FAu);                          /* 0x1636F2 */
        return 0;
    }

    Gfx_DrawCompressedTilemap(m, 0x0006B5FAu, 0x00611000u, 0u); /* 0x163700..0x16371E */
    wr16(m, obj + 0x06u, 0x0002u);                          /* 0x163722 */
    /* 0x163728: beq.b 0x163738 never taken (0x0002 != 0) */
    wr32(m, obj, 0x00163732u);                              /* 0x16372A */
    return 0;
}
