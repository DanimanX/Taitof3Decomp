/* ROM 0x16369C: delayed compressed tilemap draw tick (A0 = object).
 * Decrements +0x06 and re-arms 0x16369C while > 0. When <= 0, draws
 * compressed tilemap 0x6B7AA to 0x611800 via 0x0053B2, sets +0x06 = 2,
 * and advances to 0x1636D4. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_16369C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x16369C */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x1636A0 */
        wr32(m, obj, 0x0016369Cu);                           /* 0x163694 */
        return 0;
    }

    Gfx_DrawCompressedTilemap(m, 0x0006B7AAu, 0x00611800u, 0u); /* 0x1636A2..0x1636C0 */
    wr16(m, obj + 0x06u, 2u);                                /* 0x1636C4 (beq 1636DA never taken) */
    wr32(m, obj, 0x001636D4u);                               /* 0x1636CC */
    return 0;
}
