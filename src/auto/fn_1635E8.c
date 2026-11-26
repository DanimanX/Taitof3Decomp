/* ROM 0x1635E8: stage background tilemap flip-book phase 1 (A0 = object).
 * Counts down +0x06 and re-arms 0x1635E8 while > 0. On zero, draws compressed
 * tilemap 0x6B622 to 0x611800 via 0x0053B2, sets +0x06=2 and transitions to
 * 0x163620. */
#include "fn_auto.h"
#include "../video/sprite.h"

int Fn_1635E8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x1635E8 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x1635EC */
        wr32(m, obj, 0x001635E8u);                          /* 0x1635E0 */
        return 0;
    }

    Gfx_DrawCompressedTilemap(m, 0x0006B622u, 0x00611800u, 0u); /* 0x1635EE..0x16360C */
    wr16(m, obj + 0x06u, 2u);                               /* 0x163610 (beq 163626 never taken) */
    wr32(m, obj, 0x00163620u);                              /* 0x163618 */
    return 0;
}
