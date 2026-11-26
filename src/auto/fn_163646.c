/* ROM 0x163646: stage background tilemap flip-book phase 3 (A0 = object).
 * Counts down +0x06 and re-arms 0x163646 while > 0. On zero, draws compressed
 * tilemap 0x6B6E0 to 0x611000 via 0x0053B2, sets +0x06=2 and transitions to
 * 0x16367E. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_163646(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x163646 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x16364A */
        wr32(m, obj, 0x00163646u);                          /* 0x16363E */
        return 0;
    }

    Gfx_DrawCompressedTilemap(m, 0x0006B6E0u, 0x00611000u, 0u); /* 0x16364C..0x16366A */
    wr16(m, obj + 0x06u, 2u);                               /* 0x16366E (beq 163684 never taken) */
    wr32(m, obj, 0x0016367Eu);                              /* 0x163676 */
    return 0;
}
