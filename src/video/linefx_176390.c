/* ROM 0x176390: line-RAM perspective band. Called with D0 = band height
 * (16.16 is not used: the low word after <<7 is the row count), D7 = first
 * row, D1 = depth term, and four 0xE8-row line tables A0..A3 (zoom, scale
 * byte, x-scroll field, flags). Globals at A5+0x4298..0x42B0.
 *
 *  pass 1: bit 13 of every A3 row is set inside (first, first + count) and
 *          cleared elsewhere (rows 0xE7..0);
 *  pass 2: for rows first..min(first + count, 0xE7) (at least one row) the
 *          zoom = (0x24 - depth) * (0xFFFF6000 / (0xA0 + acc) + 0x100) << 6 /
 *          0x32 ^ 0x3F goes to A0, its scale byte to A1, and the 9-bit
 *          x step accumulator into A2; both accumulators then advance. */
#include "linefx_176390.h"

#define G_4298  (F3_A5 + 0x4298u)
#define G_429C  (F3_A5 + 0x429Cu)
#define G_42A0  (F3_A5 + 0x42A0u)
#define G_42A4  (F3_A5 + 0x42A4u)
#define G_42A8  (F3_A5 + 0x42A8u)
#define G_42AC  (F3_A5 + 0x42ACu)
#define G_42B0  (F3_A5 + 0x42B0u)

/* divs.w src,Dn: 32/16 signed; on overflow Dn is left unchanged */
static uint32_t divs_w(uint32_t dn, int16_t src)
{
    if (src == 0)
        return dn;                                      /* would trap */
    int32_t a = (int32_t)dn;
    int32_t q = a / src, r = a % src;
    if (q < -32768 || q > 32767)
        return dn;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* divu.w src,Dn: 32/16 unsigned; on overflow Dn is left unchanged */
static uint32_t divu_w(uint32_t dn, uint16_t src)
{
    if (src == 0u)
        return dn;                                      /* would trap */
    uint32_t q = dn / src, r = dn % src;
    if (q > 0xFFFFu)
        return dn;
    return (r << 16) | q;
}

static uint32_t setw(uint32_t reg, uint16_t w) { return (reg & 0xFFFF0000u) | w; }

void LineFx_176390(const f3mem_t *m, uint32_t d0, uint32_t d1, uint32_t d7,
                   uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    wr32(m, G_4298, d0);                                /* 0x176390 */
    wr16(m, G_429C, (uint16_t)d7);
    uint16_t d5 = (uint16_t)d1;
    wr32(m, G_42A0, d0 << 7);

    d0 = rd32(m, G_4298) >> 1;                          /* 0x1763A4 lsr.l */
    if (d0 != 0u)
        d1 = 0x10000u - 0x80000000u / d0;               /* divu.l ; neg.l ; addi.l */
    else
        d1 = 0xFFFFFFFFu;                               /* moveq #-1 */
    wr32(m, G_42A4, (uint32_t)-d1);                     /* 0x1763C2 neg.l */

    /* 0x1763C8: 0x10000 / (A5+0x42A0).w -- a zero word would trap */
    uint16_t cnt = rd16(m, G_42A0);
    d1 = divu_w(0x00010000u, cnt);
    wr32(m, G_42A8, (uint32_t)(uint16_t)d1 * 0x280u);   /* mulu.w */
    wr32(m, G_42AC, 0u);
    wr32(m, G_42B0, 0u);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u)
        wr32(m, G_42A4, (uint32_t)-rd32(m, G_42A4));

    /* pass 1 (0x1763F0) */
    int16_t lo = (int16_t)rd16(m, G_429C);
    int16_t hi = (int16_t)(uint16_t)(lo + rd16(m, G_42A0));
    for (int16_t r = 0xE7; r >= 0; r--) {
        uint32_t p = a3 + (uint32_t)r * 2u;
        uint16_t v = rd16(m, p);
        if (r > lo && r < hi)
            v |= 0x2000u;                               /* bset #13 */
        else
            v &= (uint16_t)~0x2000u;                    /* bclr #13 */
        wr16(m, p, v);
    }

    /* pass 2 (0x176422): do { ... } while (++row <= 0xE7 && row <= end) */
    uint16_t row = rd16(m, G_429C);
    int16_t end = (int16_t)(uint16_t)(row + rd16(m, G_42A0));
    for (;;) {
        uint16_t acc = rd16(m, G_42AC);
        uint32_t q = divs_w(0xFFFF6000u, (int16_t)(uint16_t)(0xA0u + acc));
        q = setw(q, (uint16_t)(q + 0x100u));            /* addi.w #$100 */
        uint32_t z = (uint32_t)((int32_t)(int16_t)(uint16_t)(0x24u - d5) *
                                (int32_t)(int16_t)(uint16_t)q);  /* muls.w */
        z <<= 6;                                        /* asl.l #6 */
        z = divs_w(z, 0x32);
        uint32_t off = (uint32_t)(int32_t)(int16_t)row * 2u;
        wr16(m, a0 + off, (uint16_t)(z ^ 0x3Fu));       /* eori.w ; (A0,D7.w*2) */
        wr8(m, a1 + off, (uint8_t)q);                   /* (A1,D7.w*2) */
        uint16_t x = rd16(m, a2 + off);                 /* bfins {23:9}: bits 8..0 */
        x = (uint16_t)((x & 0xFE00u) | (rd16(m, G_42B0) & 0x01FFu));
        wr16(m, a2 + off, x);
        wr32(m, G_42AC, rd32(m, G_42AC) + rd32(m, G_42A8));
        wr32(m, G_42B0, rd32(m, G_42B0) + rd32(m, G_42A4));
        row = (uint16_t)(row + 1u);
        if ((int16_t)row > 0xE7 || end < (int16_t)row)
            break;
    }
}
