/* ROM 0x163026: line-RAM palette fade ramp tick (A0 = object).
 * Computes fade level d0 = (s16)(+0x44) >> 4 (clamped to <= 8) and d1 = 4,
 * packs inverted RGB444 word ~(((d1<<8)+(8-d1))<<4 | ((d0<<8)+(8-d0))), fills
 * 0xE8 words at 0x626200 + (s16)(A5-0x5D56), decrements +0x44, re-arming
 * 0x163026 while >= 0 or advancing to 0x1630A2 when < 0. */
#include "fn_auto.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_163026(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t d0w = ASR16(rd16(m, obj + 0x44u), 4);                 /* 0x163026..0x16302A */
    int16_t d1w = 4;                                              /* 0x16302C */

    r->d[0] = setw(r->d[0], (uint16_t)d0w);
    r->d[1] = setw(r->d[1], (uint16_t)d1w);

    uint16_t c1 = (uint16_t)d1w & 0x000Fu;                        /* 0x163042 */
    if ((int16_t)c1 > 8)
        c1 = 8u;
    uint16_t c0 = (uint16_t)d0w & 0x000Fu;                        /* 0x163052 */
    if ((int16_t)c0 > 8)
        c0 = 8u;

    uint16_t p1 = (uint16_t)(((c1 << 8) + (8u - c1)) << 4);       /* 0x163062..0x16306C */
    uint16_t p0 = (uint16_t)((c0 << 8) + (8u - c0));              /* 0x16306E..0x163076 */
    uint16_t fill = (uint16_t)~(p1 | p0);                         /* 0x163078..0x16307A */

    if (m->gfx) {
        uint32_t dst = 0x00626200u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
        for (int i = 0xE7; i >= 0; i--)                           /* 0x16307C..0x163088 */
            wr16(m, dst + (uint32_t)(i * 2), fill);
    }

    int16_t cnt = (int16_t)(rd16(m, obj + 0x44u) - 1u);           /* 0x163094 */
    wr16(m, obj + 0x44u, (uint16_t)cnt);
    if (cnt >= 0) {                                               /* 0x163098 */
        wr32(m, obj, 0x00163026u);                                /* 0x16301E */
        return 0;
    }

    wr32(m, obj, 0x001630A2u);                                    /* 0x16309A */
    return 0;
}
