/* ROM 0x160814: playfield line-table fill. Reads the scroll offset at
 * A5-0x5D56 and writes 0xECCC to 0x627600+scroll and 0x02FF to
 * 0x626000+scroll over 0xE8 words (0x17669E), then replaces the low nibble
 * of the words at 0x62B400+scroll (0x8 for words 0..0x72, 0xA for
 * 0x73..0xE7), 0x62B000+scroll (0xD) and 0x62B600+scroll (0x6) through
 * 0x17668A. A0, A1, D6 and D7 are saved and restored; D0.w ends as 6 and
 * D1.w as the last word read from 0x62B600+scroll with its low nibble 6. */
#include "fn_auto.h"

/* ROM 0x17668A: D1.w = (A0,D7.w*2), replace its low nibble with D0's, store
 * back, D7.w += 1, repeat while D7.w <= D6.w (signed words). Returns D1 with
 * the last word read. */
static uint32_t fill_17668A(const f3mem_t *m, uint32_t a0, uint32_t d1,
                            uint16_t d0w, uint16_t d7w, uint16_t d6w)
{
    do {
        uint32_t addr = a0 + (uint32_t)d7w * 2u;           /* ROM 0x17668A */
        uint16_t w = rd16(m, addr);
        w = (uint16_t)((w & 0xFFF0u) | (d0w & 0x000Fu));   /* ROM 0x17668E bfins D0,D1{28:4} */
        d1 = (d1 & 0xFFFF0000u) | w;                       /* ROM 0x17668E */
        wr16(m, addr, w);                                  /* ROM 0x176692 */
        d7w++;                                             /* ROM 0x176696 */
    } while ((int16_t)d7w <= (int16_t)d6w);                /* ROM 0x176698 cmp/ble */
    return d1;
}

/* ROM 0x17669E: store D0.w at (A0,D7.w*2) and D1.w at (A1,D7.w*2), D7.w +=
 * 1, repeat while D7.w <= D6.w (signed words). */
static void fill_17669E(const f3mem_t *m, uint32_t a0, uint32_t a1,
                        uint16_t d0w, uint16_t d1w, uint16_t d7w, uint16_t d6w)
{
    do {
        wr16(m, a0 + (uint32_t)d7w * 2u, d0w);             /* ROM 0x17669E */
        wr16(m, a1 + (uint32_t)d7w * 2u, d1w);             /* ROM 0x1766A2 */
        d7w++;                                             /* ROM 0x1766A6 */
    } while ((int16_t)d7w <= (int16_t)d6w);                /* ROM 0x1766A8 cmp/ble */
}

int Fn_160814(const f3mem_t *m, f3regs_t *r)
{
    int16_t scroll = (int16_t)rd16(m, F3_A5 - 0x5D56u);   /* 0x16081E adda.w (-0x5d56,A5),A0 */
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    /* 0x160818..0x160842: A0 = 0x627600+s, A1 = 0x626000+s; pushed 0, 0xE7,
     * 0x2FF pop into D7 = 0, D6 = 0xE7, D1 = 0x2FF; D0.w = 0xECCC */
    d0 = (d0 & 0xFFFF0000u) | 0xECCCu;
    d1 = (d1 & 0xFFFF0000u) | 0x02FFu;
    fill_17669E(m, 0x00627600u + (uint32_t)(int32_t)scroll,
                0x00626000u + (uint32_t)(int32_t)scroll,
                (uint16_t)d0, (uint16_t)d1, 0x0000u, 0x00E7u);

    /* 0x16084C..0x16086A: A0 = 0x62B400+s, D0.w = 8, D7.w = 0, D6.w = 0x72 */
    d0 = (d0 & 0xFFFF0000u) | 0x0008u;
    d1 = fill_17668A(m, 0x0062B400u + (uint32_t)(int32_t)scroll, d1,
                     (uint16_t)d0, 0x0000u, 0x0072u);

    /* 0x160874..0x160892: A0 = 0x62B400+s, D0.w = 0xA, D7.w = 0x73, D6.w = 0xE7 */
    d0 = (d0 & 0xFFFF0000u) | 0x000Au;
    d1 = fill_17668A(m, 0x0062B400u + (uint32_t)(int32_t)scroll, d1,
                     (uint16_t)d0, 0x0073u, 0x00E7u);

    /* 0x16089C..0x1608BA: A0 = 0x62B000+s, D0.w = 0xD, D7.w = 0, D6.w = 0xE7 */
    d0 = (d0 & 0xFFFF0000u) | 0x000Du;
    d1 = fill_17668A(m, 0x0062B000u + (uint32_t)(int32_t)scroll, d1,
                     (uint16_t)d0, 0x0000u, 0x00E7u);

    /* 0x1608C4..0x1608E2: A0 = 0x62B600+s, D0.w = 6, D7.w = 0, D6.w = 0xE7 */
    d0 = (d0 & 0xFFFF0000u) | 0x0006u;
    d1 = fill_17668A(m, 0x0062B600u + (uint32_t)(int32_t)scroll, d1,
                     (uint16_t)d0, 0x0000u, 0x00E7u);

    r->d[0] = d0;                                          /* 0x1608DA move.w #6,D0 */
    r->d[1] = d1;                                          /* last 17668A word */
    /* 0x160848/0x160870/0x160898/0x1608C0/0x1608E8 movem restore D6, D7,
     * A0 and A1 to their entry values; 0x1608EC rts */
    return 0;
}
