/* ROM 0x11F0A0: screen-shake countdown tick (A0 = object).
 * When A5+0x1D58 bit 1 is set, jitters A5+0x30D2 by (rng & 3) - 1 toward 0xA0.
 * Decrements +0x06 and re-arms 0x11F0A0 while > 0. Once <= 0, sets A5+0x30EE
 * bit 12 (byte 0x30EF |= 0x08), plays hooked sound 0x27, sets +0x06 = 0x1E,
 * and transitions to 0x11F0F4. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

int Fn_11F0A0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x11F0A0..0x11F0A6 */
        uint32_t d0 = Rng_NextMem(m);                    /* 0x11F0A8 */
        uint16_t d0w = (uint16_t)((d0 & 0x0003u) - 1u);  /* 0x11F0AE..0x11F0B2 */
        r->d[0] = (d0 & 0xFFFF0000u) | d0w;
        if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x00A0)  /* 0x11F0B4..0x11F0BA ble */
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) - d0w)); /* 0x11F0BC */
        else
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + d0w)); /* 0x11F0C2 */
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x11F0C6 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                       /* 0x11F0CA bgt */
        wr32(m, obj + 0x00u, 0x0011F0A0u);               /* 0x11F098 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x08u)); /* 0x11F0CC bfset {12:1} */
    Sound_PlayHooked(m, 0x0027u);                        /* 0x11F0D2..0x11F0E2 */
    wr16(m, obj + 0x06u, 0x001Eu);                       /* 0x11F0E6 */
    wr32(m, obj + 0x00u, 0x0011F0F4u);                   /* 0x11F0EC */
    return 0;
}
