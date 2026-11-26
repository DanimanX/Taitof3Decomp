/* ROM 0x11F84C: boss intro/attack phase timer tick (A0 = object).
 * Decrements +0x06 while > 0 (re-arming 0x11F84C). On 0, sets A5+0x30EE{5:1}
 * (bit 2 of byte A5+0x30EE), sets +0x06 = 0xF0 (0x11F85E beq folded), and
 * arms 0x11F868. */
#include "fn_auto.h"

int Fn_11F84C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11F84C */
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {                                /* 0x11F850 -> 0x11F844 */
        wr32(m, obj, 0x0011F84Cu);
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x04u)); /* 0x11F852 {5:1} */
    wr16(m, obj + 0x06u, 0x00F0u);                       /* 0x11F858 (0x11F85E beq folded) */
    wr32(m, obj, 0x0011F868u);                           /* 0x11F860 */
    return 0;
}
