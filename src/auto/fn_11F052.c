/* ROM 0x11F052: screen-shake countdown and transition tick (A0 = object).
 * When A5+0x1D58 bit 1 is set, jitters A5+0x30D2 by (Rng_Next & 3) - 1 toward
 * 0xA0. Decrements +0x06 and re-arms 0x11F052 while > 0. Once <= 0, sets
 * A5+0x30F2 = 2, computes +0x06 = 0x2BC - 0x0CBEB0(0x190), and advances to
 * 0x11F0A0. */
#include "fn_auto.h"

int Fn_11F052(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x11F052..0x11F058 */
        d0 = Rng_NextMem(m);                             /* 0x11F05A */
        uint16_t d0w = (uint16_t)((d0 & 0x0003u) - 1u);  /* 0x11F060..0x11F064 */
        d0 = (d0 & 0xFFFF0000u) | d0w;
        if ((int16_t)rd16(m, F3_A5 + 0x30D2u) > 0x00A0)  /* 0x11F066..0x11F06C */
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) - d0w)); /* 0x11F06E */
        else
            wr16(m, F3_A5 + 0x30D2u, (uint16_t)(rd16(m, F3_A5 + 0x30D2u) + d0w)); /* 0x11F074 */
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x11F078 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                       /* 0x11F07C */
        wr32(m, obj, 0x0011F052u);                       /* 0x11F04A */
        r->d[0] = d0;
        return 0;
    }

    wr8(m, F3_A5 + 0x30F2u, 2u);                         /* 0x11F07E */

    /* ROM 0x0CBEB0(0x0190) */
    uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0..0x0CBEB6 */
    d1 = (d1 & 0xFFFF0000u) | d1w;
    uint32_t prod = (uint32_t)((int32_t)(int16_t)d1w * (int32_t)0x0190) << 4; /* 0x0CBEBA..0x0CBEBC */
    d0 = (prod << 16) | (prod >> 16);                    /* 0x0CBEBE swap D0 */

    uint16_t w = (uint16_t)((uint16_t)(0u - (uint16_t)d0) + 0x02BCu); /* 0x11F08E..0x11F090 */
    d0 = (d0 & 0xFFFF0000u) | w;
    wr16(m, obj + 0x06u, w);                             /* 0x11F094 */
    wr32(m, obj, 0x0011F0A0u);                           /* 0x11F098 */

    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
