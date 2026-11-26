/* ROM 0x163732: stage background tilemap flip-book phase 7 (A0 = object).
 * Decrements +0x06 and re-arms 0x163732 while > 0. On <= 0, branches to
 * 0x1635D0 to clear A5+0x4210.l and A5-0x5D9C.l, set +0x06 = 2, and loop back
 * to 0x1635E8 (0x1635DE beq folded since 2 != 0). */
#include "fn_auto.h"

int Fn_163732(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x163732 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x163736 -> 0x16372A */
        wr32(m, obj, 0x00163732u);
        return 0;
    }

    wr32(m, F3_A5 + 0x4210u, 0u);                           /* 0x1635D0 */
    wr32(m, F3_A5 - 0x5D9Cu, 0u);                           /* 0x1635D4 */
    wr16(m, obj + 0x06u, 2u);                               /* 0x1635D8 (0x1635DE beq folded) */
    wr32(m, obj, 0x001635E8u);                              /* 0x1635E0 */
    return 0;                                               /* 0x1635E6 */
}
