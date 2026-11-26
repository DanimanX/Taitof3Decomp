/* ROM 0x11F9AA: boss shutter step-down countdown tick (A0 = object).
 * Counts down +0x06 and re-arms 0x11F9AA while > 0. On <= 0, if (int8_t)A5+0x3262
 * > 0, decrements A5+0x3262, reloads +0x06 = 10, and re-arms 0x11F9AA; else sets
 * A5+0x30EE{1:1} (0x40), clears A5+0x30EE{11:1} (0x30EF bit 0x10), and arms 0x11F7BE. */
#include "fn_auto.h"

int Fn_11F9AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11F9AA */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                              /* 0x11F9AE */
        wr32(m, obj, 0x0011F9AAu);                       /* 0x11F9A2 */
        return 0;
    }

    if ((int8_t)rd8(m, F3_A5 + 0x3262u) > 0) {           /* 0x11F9B0..0x11F9B6 */
        wr8(m, F3_A5 + 0x3262u, (uint8_t)(rd8(m, F3_A5 + 0x3262u) - 1u)); /* 0x11F996 */
        wr16(m, obj + 0x06u, 0x000Au);                   /* 0x11F99A (0x11F9A0 beq folded) */
        wr32(m, obj, 0x0011F9AAu);                       /* 0x11F9A2 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x40u));  /* 0x11F9B8 {1:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & ~0x10u)); /* 0x11F9BE {11:1} */
    wr32(m, obj, 0x0011F7BEu);                           /* 0x11F9C4 -> 0x11F7B6 */
    return 0;
}
