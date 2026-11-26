/* ROM 0x121B80: boss phase timer tick (A0 = object).
 * If (int8_t)(A5+0x326E) >= 6, transitions via shared 0x121D54 block (plays
 * hooked sound 0x13, clears A5+0x30EE.l, computes +0x06, randomizes +0x3A/+0x3C,
 * arms 0x121DAC). Otherwise decrements +0x06, re-arming 0x121B80 while > 0,
 * else sets A5+0x30EE{9:1} (A5+0x30EF bit 6), +0x20=0x8000, +0x42=3, +0x06=0x300,
 * and arms 0x121BB0. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_121B80(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) >= 6) {                      /* 0x121B80..0x121B86 */
        Sound_PlayHooked(m, 0x0013u);                                /* 0x121D54..0x121D64 */
        wr32(m, F3_A5 + 0x30EEu, 0u);                                /* 0x121D68 */

        uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0 */
        uint32_t d0 = (uint32_t)((int32_t)(int16_t)0x003C * (int32_t)(int16_t)d1w) << 4;
        d0 = (d0 << 16) | (d0 >> 16);
        uint16_t cnt = (uint16_t)(0u - (uint16_t)((uint16_t)d0 - 0x0078u)); /* 0x121D76..0x121D7A */
        wr16(m, obj + 0x06u, cnt);                                   /* 0x121D7C */

        uint32_t rng1 = Rng_NextMem(m);                              /* 0x121D80 */
        wr16(m, obj + 0x3Au, (uint16_t)((rng1 & 0x001Fu) + 0x00CDu)); /* 0x121D86..0x121D8E */

        uint32_t rng2 = Rng_NextMem(m);                              /* 0x121D92 */
        uint16_t y = (uint16_t)((rng2 & 0x007Fu) + 0x0040u);         /* 0x121D98..0x121D9C */
        wr16(m, obj + 0x3Cu, y);                                     /* 0x121DA0 */

        wr32(m, obj, 0x00121DACu);                                   /* 0x121DA4 */
        r->d[0] = setw(rng2, y);
        r->d[1] = setw(r->d[1], d1w);
        return 0;
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);            /* 0x121B8A */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                          /* 0x121B8E */
        wr32(m, obj, 0x00121B80u);                                   /* 0x121B78 */
        return 0;
    }

    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) | 0x40u)); /* 0x121B90 {9:1} */
    wr16(m, obj + 0x20u, 0x8000u);                                   /* 0x121B96 */
    wr16(m, obj + 0x42u, 0x0003u);                                   /* 0x121B9C */
    wr16(m, obj + 0x06u, 0x0300u);                                   /* 0x121BA2 */
    wr32(m, obj, 0x00121BB0u);                                       /* 0x121BA8 */
    return 0;
}
