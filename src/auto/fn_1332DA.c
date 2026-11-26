/* ROM 0x1332DA: boss stage-scroll retreat tick (A0 = object).
 * Subtracts 0xC000 per frame from A5+0x30D2 and re-arms 0x1332DA while high
 * word > 0x20. Once <= 0x20, sets A5+0x30F2 = 5, sets A5+0x30EE bits 12, 0, 8,
 * plays sound 0x2C via Sound_PlayHooked, sets +0x06 = 0x78, and advances to
 * 0x133326. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_1332DA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - 0x0000C000u); /* 0x1332DA */
    if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x0020) {                 /* 0x1332E2 */
        wr32(m, obj, 0x001332DAu);                                    /* 0x1332D2 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 5u);                                      /* 0x1332EA */
    /* 0x1332F0: bfset (0x30ee,A5){0xc:1} */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u));

    Sound_PlayHooked(m, 0x002Cu);                                     /* 0x1332F6..0x133306 */

    /* 0x13330A: bfset (0x30ee,A5){0x0:1}, 0x133310: bfset (0x30ee,A5){0x8:1} */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u));
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x80u));

    wr16(m, obj + 0x06u, 0x0078u);                                    /* 0x133316 (0x13331C beq never taken) */
    wr32(m, obj, 0x00133326u);                                        /* 0x13331E */
    return 0;
}
