/* ROM 0x11CCB8: camera/boss vertical approach scroll tick (A0 = object).
 * Pans camera scroll (A5+0x30D2, A5+0x30D6) by (-0x4000, -0xC000) per frame
 * and re-arms 0x11CCB8 while (int16_t)A5+0x30D6 > 0x70. Once <= 0x70, clears
 * A5+0x30F3, plays hooked sound 0x13, sets +0x3A/+0x20 = 0xBC00, +0x06 = 0xF0,
 * sets A5+0x30EE bit 5 (0x04), sets +0x42 = 6, and transitions to 0x11CD0E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_11CCB8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, F3_A5 + 0x30D2u, rd32(m, F3_A5 + 0x30D2u) - 0x00004000u); /* 0x11CCB8 */
    wr32(m, F3_A5 + 0x30D6u, rd32(m, F3_A5 + 0x30D6u) - 0x0000C000u); /* 0x11CCC0 */

    if ((int16_t)rd16(m, F3_A5 + 0x30D6u) > 0x0070) {    /* 0x11CCC8..0x11CCCE bgt */
        wr32(m, obj + 0x00u, 0x0011CCB8u);               /* 0x11CCB0 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30F3u, 0u);                         /* 0x11CCD0 */
    Sound_PlayHooked(m, 0x0013u);                        /* 0x11CCD4..0x11CCE4 */
    wr16(m, obj + 0x3Au, 0xBC00u);                       /* 0x11CCE8 */
    wr16(m, obj + 0x20u, rd16(m, obj + 0x3Au));          /* 0x11CCEE */
    wr16(m, obj + 0x06u, 0x00F0u);                       /* 0x11CCF4 */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x04u)); /* 0x11CCFA bfset {5:1} */
    wr16(m, obj + 0x42u, 0x0006u);                       /* 0x11CD00 */
    wr32(m, obj + 0x00u, 0x0011CD0Eu);                   /* 0x11CD06 */
    return 0;
}
