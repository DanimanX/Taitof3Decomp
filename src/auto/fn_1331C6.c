/* ROM 0x1331C6: boss stage-scroll entrance tick (A0 = object).
 * Advances shared boss X position A5+0x30D2 by 0xC000 per frame; while high
 * word < 0x120 re-arms 0x1331C6. Once >= 0x120, sets A5+0x30F2 = 5, sets
 * bits 12, 4, 0, 8 of A5+0x30EE, plays sound 0x2C via Sound_PlayHooked, sets
 * +0x06 = 0x78, and advances to 0x133218. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_1331C6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) + 0x0000C000u); /* 0x1331C6 */

    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) < 0x0120) {       /* 0x1331CE..0x1331D4 */
        wr32(m, obj, 0x001331C6u);                          /* 0x1331BE */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 5u);                            /* 0x1331D6 */
    /* 0x1331DC: bfset (0x30ee,A5){0xc:1}, 0x1331E2: bfset (0x30ee,A5){0x4:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x08u));

    Sound_PlayHooked(m, 0x002Cu);                           /* 0x1331E8..0x1331F8 */

    /* 0x1331FC: bfset (0x30ee,A5){0x0:1}, 0x133202: bfset (0x30ee,A5){0x8:1} */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u));
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x80u));

    wr16(m, obj + 0x06u, 0x0078u);                          /* 0x133208 (beq 13321E never taken) */
    wr32(m, obj, 0x00133218u);                              /* 0x133210 */
    return 0;
}
