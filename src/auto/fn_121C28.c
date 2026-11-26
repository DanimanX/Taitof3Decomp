/* ROM 0x121C28: boss/stage phase transition countdown tick (A0 = object).
 * If (int8_t)(A5+0x326E) >= 6, plays hooked sound 0x13, clears A5+0x30EE.l,
 * computes +0x06 = 0x78 - 0x0CBEB0(0x3C), randomizes +0x3A/+0x3C via
 * Rng_NextMem (0x001D8A), and transitions to 0x121DAC. Otherwise decrements
 * +0x06, re-arming 0x121C28 while > 0, else clears player+0x58 on both
 * players (0x123630), updates A5+0x30EE bits {9:1}=0 and {0:1}=1, sets
 * +0x06 = 0x1E, and advances to 0x121C60. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_121C28(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) >= 6) {              /* 0x121C28..0x121C2E */
        Sound_PlayHooked(m, 0x0013u);                        /* 0x121D54..0x121D64 */
        wr32(m, F3_A5 + 0x30EEu, 0u);                        /* 0x121D68 */

        uint16_t d1w = (uint16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0 */
        uint32_t d0 = (uint32_t)((int32_t)(int16_t)0x003C * (int32_t)(int16_t)d1w) << 4;
        d0 = (d0 << 16) | (d0 >> 16);
        uint16_t cnt = (uint16_t)(0u - (uint16_t)((uint16_t)d0 - 0x0078u)); /* 0x121D76..0x121D7A */
        wr16(m, obj + 0x06u, cnt);                           /* 0x121D7C */

        uint32_t rng1 = Rng_NextMem(m);                      /* 0x121D80 */
        wr16(m, obj + 0x3Au, (uint16_t)((rng1 & 0x001Fu) + 0x00CDu)); /* 0x121D86..0x121D8E */

        uint32_t rng2 = Rng_NextMem(m);                      /* 0x121D92 */
        uint16_t y = (uint16_t)((rng2 & 0x007Fu) + 0x0040u); /* 0x121D98..0x121D9C */
        wr16(m, obj + 0x3Cu, y);                             /* 0x121DA0 */

        wr32(m, obj, 0x00121DACu);                           /* 0x121DA4 */
        r->d[0] = setw(rng2, y);
        r->d[1] = setw(r->d[1], d1w);
        return 0;
    }

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);    /* 0x121C32 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                                  /* 0x121C36 */
        wr32(m, obj, 0x00121C28u);                           /* 0x121C20 */
        return 0;
    }

    r->d[0] &= 0xFFFFFF00u;                                  /* 0x121C38 */
    r->d[1] &= 0xFFFFFF00u;                                  /* 0x121C3C */
    wr8(m, rd32(m, F3_A5 + 0x3264u) + 0x58u, 0u);            /* 0x121C40 -> 0x123630 */
    uint32_t p2 = rd32(m, F3_A5 + 0x3268u);
    wr8(m, p2 + 0x58u, 0u);
    r->a[1] = p2;

    wr8(m, F3_A5 + 0x30EFu, (uint8_t)(rd8(m, F3_A5 + 0x30EFu) & ~0x40u)); /* 0x121C44 {9:1} */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u));  /* 0x121C4A {0:1} */
    wr16(m, obj + 0x06u, 0x001Eu);                           /* 0x121C50 (beq 121C66 never taken) */
    wr32(m, obj, 0x00121C60u);                               /* 0x121C58 */
    return 0;
}
