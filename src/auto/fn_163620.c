/* ROM 0x163620: stage background tilemap flip-book phase 2 (A0 = object).
 * Counts down +0x06 and re-arms 0x163620 while > 0. On <= 0, sets
 * A5+0x4210 = 0x01000000 and A5-0x5D9C = 0x01000000, sets +0x06 = 2, and
 * transitions to 0x163646. */
#include "fn_auto.h"

int Fn_163620(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x163620 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x163624 */
        wr32(m, obj, 0x00163620u);                           /* 0x163618 */
        return 0;
    }

    wr32(m, F3_A5 + 0x4210u, 0x01000000u);                   /* 0x163626 */
    wr32(m, F3_A5 - 0x5D9Cu, 0x01000000u);                   /* 0x16362E */
    wr16(m, obj + 0x06u, 2u);                                /* 0x163636 (beq 0x16364C never taken) */
    wr32(m, obj, 0x00163646u);                               /* 0x16363E */
    return 0;
}
