/* ROM 0x1636D4: stage background tilemap flip-book phase 5 (A0 = object).
 * Decrements +0x06 and re-arms 0x1636D4 while > 0. On <= 0, sets
 * A5+0x4210.l and A5-0x5D9C.l to 0x1000000, sets +0x06 = 2, and transitions
 * to 0x1636FA. */
#include "fn_auto.h"

int Fn_1636D4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);   /* 0x1636D4 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                 /* 0x1636D8 -> 0x1636CC */
        wr32(m, obj, 0x001636D4u);
        return 0;
    }

    wr32(m, F3_A5 + 0x4210u, 0x01000000u);                  /* 0x1636DA */
    wr32(m, F3_A5 - 0x5D9Cu, 0x01000000u);                  /* 0x1636E2 */
    wr16(m, obj + 0x06u, 2u);                               /* 0x1636EA (0x1636F0 beq folded) */
    wr32(m, obj, 0x001636FAu);                              /* 0x1636F2 */
    return 0;
}
