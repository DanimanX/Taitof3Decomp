/* ROM 0x121D40: stage/boss phase gate tick (A0 = object).
 * While (int8_t)A5+0x326E < 6: counts down +0x06 and re-arms 0x121D40; once
 * <= 0, clears A5+0x3264->+0x58 and A5+0x3268->+0x58 via 0x123630, sets
 * A5+0x30EE{0:1} (0x80), sets +0x06 = 0x1E and arms 0x121B60. Once A5+0x326E
 * >= 6: plays hooked sound 0x13, clears A5+0x30EE.l, computes +0x06 = 0x78 -
 * 0x0CBEB0(0x3C), picks random +0x3A in [0xCD..0xEC] and +0x3C in [0x40..0xBF]
 * via Rng_NextMem (0x001D8A), and arms 0x121DAC. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x123630: store D0.b / D1.b into +0x58 of A5+0x3264 / A5+0x3268. */
static void sub_123630(const f3mem_t *m, f3regs_t *r, uint8_t d0b, uint8_t d1b)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x3264u);              /* 0x123630 */
    wr8(m, a1 + 0x58u, d0b);                             /* 0x123634 */
    a1 = rd32(m, F3_A5 + 0x3268u);                       /* 0x123638 */
    wr8(m, a1 + 0x58u, d1b);                             /* 0x12363C */
    r->a[1] = a1;
}

/* ROM 0x0CBEB0: difficulty-scaled speed word from A5+0x087E. */
static uint32_t sub_0CBEB0(const f3mem_t *m, f3regs_t *r, uint16_t d0w)
{
    int16_t d1w = (int16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u); /* 0x0CBEB0..0x0CBEB6 */
    r->d[1] = setw(r->d[1], (uint16_t)d1w);
    uint32_t prod = (uint32_t)((int32_t)(int16_t)d0w * (int32_t)d1w); /* 0x0CBEBA */
    prod <<= 4;                                          /* 0x0CBEBC asl.l #4 */
    return (prod << 16) | (prod >> 16);                  /* 0x0CBEBE swap */
}

int Fn_121D40(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int8_t)rd8(m, F3_A5 + 0x326Eu) < 6) {           /* 0x121D40..0x121D46 */
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x121D4A */
        wr16(m, obj + 0x06u, cnt);
        if ((int16_t)cnt > 0) {                          /* 0x121D4E -> 0x121D38 */
            wr32(m, obj + 0x00u, 0x00121D40u);
            return 0;
        }
        /* 0x121D50 -> 0x121B3E */
        r->d[0] &= 0xFFFFFF00u;                          /* 0x121B3E */
        r->d[1] &= 0xFFFFFF00u;                          /* 0x121B42 */
        sub_123630(m, r, 0u, 0u);                        /* 0x121B46 */
        wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u)); /* 0x121B4A */
        wr16(m, obj + 0x06u, 0x001Eu);                   /* 0x121B50 (0x121B56 beq folded) */
        wr32(m, obj + 0x00u, 0x00121B60u);               /* 0x121B58 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0013u);                        /* 0x121D54..0x121D64 */
    wr32(m, F3_A5 + 0x30EEu, 0u);                        /* 0x121D68 */

    {
        uint32_t d0 = sub_0CBEB0(m, r, 0x003Cu);         /* 0x121D6C..0x121D70 */
        uint16_t d0w = (uint16_t)((uint16_t)d0 - 0x0078u); /* 0x121D76 */
        d0w = (uint16_t)(0u - d0w);                      /* 0x121D7A neg.w */
        wr16(m, obj + 0x06u, d0w);                       /* 0x121D7C */
    }

    {
        uint32_t d0 = Rng_NextMem(m);                    /* 0x121D80 */
        uint16_t d0w = (uint16_t)((d0 & 0x001Fu) + 0x00CDu); /* 0x121D86..0x121D8A */
        wr16(m, obj + 0x3Au, d0w);                       /* 0x121D8E */
    }

    {
        uint32_t d0 = Rng_NextMem(m);                    /* 0x121D92 */
        uint16_t d0w = (uint16_t)((d0 & 0x007Fu) + 0x0040u); /* 0x121D98..0x121D9C */
        wr16(m, obj + 0x3Cu, d0w);                       /* 0x121DA0 */
        r->d[0] = setw(d0, d0w);
    }

    wr32(m, obj + 0x00u, 0x00121DACu);                   /* 0x121DA4 */
    return 0;
}
