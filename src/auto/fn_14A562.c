/* ROM 0x14A562: repaint the line-scroll tables. Clears the low nibble of the
 * 0xE8 words at 0x62B000+scroll (0x17668A with D0 = 8), 0x62B400+scroll
 * (D0 = 6) and 0x62B600+scroll (D0 = 0xA), then writes 0xEEEE to
 * 0x627600+scroll and 0x02FF to 0x626000+scroll through 0x17669E. A0, A1, D6
 * and D7 are saved and restored; D0.w ends as 0xEEEE (the last value poked)
 * and D1.w as 0x02FF. */
#include "fn_auto.h"

/* ROM 0x17668A: D1.w = (A0,D7.w*2), replace its low nibble with D0's, store
 * back, until D7 passes D6 (both words compared signed). */
static void fill_17668A(const f3mem_t *m, uint32_t a0, uint16_t d0w, uint16_t d6w)
{
    for (uint16_t d7 = 0u; (int16_t)d7 <= (int16_t)d6w; d7++) {   /* ROM 0x17668A..0x17669A */
        uint32_t addr = a0 + (uint32_t)d7 * 2u;
        uint16_t w = rd16(m, addr);
        w = (uint16_t)((w & 0xFFF0u) | (d0w & 0x000Fu));           /* ROM 0x17668E bfins D0,D1{28:4} */
        wr16(m, addr, w);                                          /* ROM 0x176692 */
    }
}

/* ROM 0x17669E: store D0.w at (A0,D7.w*2) and D1.w at (A1,D7.w*2) until D7
 * passes D6. */
static void fill_17669E(const f3mem_t *m, uint32_t a0, uint32_t a1,
                        uint16_t d0w, uint16_t d1w, uint16_t d6w)
{
    for (uint16_t d7 = 0u; (int16_t)d7 <= (int16_t)d6w; d7++) {   /* ROM 0x17669E..0x1766AA */
        wr16(m, a0 + (uint32_t)d7 * 2u, d0w);
        wr16(m, a1 + (uint32_t)d7 * 2u, d1w);
    }
}

int Fn_14A562(const f3mem_t *m, f3regs_t *r)
{
    int16_t scroll = (int16_t)rd16(m, F3_A5 - 0x5D56u);           /* adda.w (-0x5D56,A5),A0 */

    fill_17668A(m, 0x0062B000u + (uint32_t)(int32_t)scroll, 0x0008u, 0x00E7u); /* ROM 0x14A562..0x14A580 */
    fill_17668A(m, 0x0062B400u + (uint32_t)(int32_t)scroll, 0x0006u, 0x00E7u); /* ROM 0x14A58A..0x14A5A8 */
    fill_17668A(m, 0x0062B600u + (uint32_t)(int32_t)scroll, 0x000Au, 0x00E7u); /* ROM 0x14A5B2..0x14A5D0 */
    fill_17669E(m, 0x00627600u + (uint32_t)(int32_t)scroll,         /* ROM 0x14A5DA..0x14A608 */
                0x00626000u + (uint32_t)(int32_t)scroll,
                0xEEEEu, 0x02FFu, 0x00E7u);

    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0xEEEEu;  /* move.w #-0x1112,D0.w */
    r->d[1] = (r->d[1] & 0xFFFF0000u) | 0x02FFu;  /* move.w (SP)+,D1.w */
    return 0;                                     /* ROM 0x14A612 rts */
}
