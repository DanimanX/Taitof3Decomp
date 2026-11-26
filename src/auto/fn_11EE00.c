/* ROM 0x11EE00: boss intro delay & camera setup tick (A0 = object).
 * Counts down +0x06 and re-arms 0x11EE00 while > 0. Once <= 0, sets camera
 * position (A5+0x30D2 = 0x1AE0000, A5+0x30D6 = 0xA40000), sets A5+0x30EE bit
 * 12 (0x30EF |= 0x08), copies camera pos to (+0x14, +0x18), sets +0x20 =
 * -0x4000, +0x42 = 0x100, +0x06 = 5, and transitions to 0x11EE42. */
#include "fn_auto.h"

int Fn_11EE00(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x11EE00 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                       /* 0x11EE04 */
        wr32(m, obj, 0x0011EE00u);                       /* 0x11EDF8 */
        return 0;
    }

    wr32(m, F3_A5 + 0x30D2u, 0x01AE0000u);               /* 0x11EE06 */
    wr32(m, F3_A5 + 0x30D6u, 0x00A40000u);               /* 0x11EE0E */
    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x11EE16 bfset {12:1} */
    wr32(m, obj + 0x14u, rd32(m, F3_A5 + 0x30D2u));      /* 0x11EE1C */
    wr32(m, obj + 0x18u, rd32(m, F3_A5 + 0x30D6u));      /* 0x11EE22 */
    wr16(m, obj + 0x20u, 0xC000u);                       /* 0x11EE28 */
    wr16(m, obj + 0x42u, 0x0100u);                       /* 0x11EE2E */
    wr16(m, obj + 0x06u, 0x0005u);                       /* 0x11EE34 */
    wr32(m, obj, 0x0011EE42u);                           /* 0x11EE3A */
    return 0;
}
