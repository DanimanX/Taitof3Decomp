/* ROM 0x148C26: stage-clear flash fill. Given the scroll offset at A5-0x5D56,
 * writes 0x9CCC/0x02FF over 0xE8 words of 0x627600+scroll and 0x626000+scroll
 * (0x17669E), then replaces the low nibble of 0xE8 words at 0x62B000+scroll,
 * 0x62B200+scroll and 0x62B400+scroll with 0xE, 0xA and 0x8 (0x17668A). */
#include "fn_auto.h"

/* ROM 0x17669E: A0[i] = D0.w, A1[i] = D1.w for i = D7.w .. D6.w */
static void fill_pairs(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint16_t d0, uint16_t d1, uint16_t d6)
{
    for (uint16_t i = 0u; (int16_t)i <= (int16_t)d6; i++) {
        wr16(m, a0 + 2u * (uint32_t)i, d0);
        wr16(m, a1 + 2u * (uint32_t)i, d1);
    }
}

/* ROM 0x17668A: low nibble of A0[i] = D0.b for i = D7.w .. D6.w; leaves the
 * last word it read in D1.w */
static uint32_t fill_nibble(const f3mem_t *m, uint32_t a0, uint32_t d0,
                            uint32_t d1, uint16_t d6)
{
    for (uint16_t i = 0u; (int16_t)i <= (int16_t)d6; i++) {
        uint16_t w = rd16(m, a0 + 2u * (uint32_t)i);
        w = (uint16_t)((w & 0xFFF0u) | (d0 & 0x0Fu));        /* bfins D0,D1{0x1C:4} */
        wr16(m, a0 + 2u * (uint32_t)i, w);
        d1 = (d1 & 0xFFFF0000u) | w;
    }
    return d1;
}

int Fn_148C26(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t scroll = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint16_t last = 0x00E7u;                 /* D6.w of all four calls */

    /* 0x148C2A..0x148C54: A0 = 0x627600+s, A1 = 0x626000+s, D0.w = -0x6334
     * (0x9CCC), D1.w = 0x2FF (pushed 0, 0xE7, 0x2FF then popped into
     * D1/D6/D7), D6.w = 0xE7, D7.w = 0; jsr 0x17669E */
    d0 = (d0 & 0xFFFF0000u) | 0x9CCCu;
    d1 = (d1 & 0xFFFF0000u) | 0x02FFu;
    fill_pairs(m, 0x627600u + scroll, 0x626000u + scroll,
               (uint16_t)d0, (uint16_t)d1, last);

    /* 0x148C62..0x148C7C */
    d0 = (d0 & 0xFFFF0000u) | 0x000Eu;
    d1 = fill_nibble(m, 0x62B000u + scroll, d0, d1, last);

    /* 0x148C8A..0x148CA4 */
    d0 = (d0 & 0xFFFF0000u) | 0x000Au;
    d1 = fill_nibble(m, 0x62B200u + scroll, d0, d1, last);

    /* 0x148CB2..0x148CCC */
    d0 = (d0 & 0xFFFF0000u) | 0x0008u;
    d1 = fill_nibble(m, 0x62B400u + scroll, d0, d1, last);

    /* 0x148CD2..0x148CD6: movem restores D6/D7/A0[/A1]; D0 and D1 keep the
     * values the last loop left (D0.w = 8, D1.w = the last word read). */
    r->d[0] = d0;
    r->d[1] = d1;
    return 0;
}
