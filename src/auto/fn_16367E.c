/* ROM 0x16367E: stage background tilemap flip-book phase 4 (A0 = object).
 * Decrements +0x06 and re-arms 0x16367E while > 0. On <= 0, clears A5+0x4210.l
 * and A5-0x5D9C.l, sets +0x06 = 2, and transitions to 0x16369C (0x163692 beq
 * folded since 2 != 0). */
#include "fn_auto.h"

int Fn_16367E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x16367E */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x163682 -> 0x163676 */
        wr32(m, obj, 0x0016367Eu);
        return 0;
    }

    wr32(m, F3_A5 + 0x4210u, 0u);                           /* 0x163684 */
    wr32(m, F3_A5 - 0x5D9Cu, 0u);                           /* 0x163688 */
    wr16(m, obj + 0x06u, 2u);                               /* 0x16368C (0x163692 beq folded) */
    wr32(m, obj, 0x0016369Cu);                              /* 0x163694 */
    return 0;                                               /* 0x16369A */
}
