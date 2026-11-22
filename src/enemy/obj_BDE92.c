#include "obj_BDE92.h"

/* 68k divs.w: (remainder << 16) | quotient; on overflow the destination is
 * unchanged (V set). Divisor is never 0 here. */
static uint32_t divs_w(uint32_t n, uint16_t d)
{
    int32_t q = (int32_t)n / (int32_t)(int16_t)d;
    int32_t r = (int32_t)n % (int32_t)(int16_t)d;
    if (q < -32768 || q > 32767)
        return n;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* ROM 0x176832: line-RAM zoom ramp. Lines D7..D6 of the tables at A0
 * (word, 0x3F ^ zoom) and A1 (byte, scale), both offset by A5-0x5D56. */
static void sub_176832(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint16_t d7w, uint16_t d6w, uint32_t v4298,
                       uint32_t d5, uint32_t d3, uint16_t v42a0)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    a0 += off;
    a1 += off;
    int16_t d7 = (int16_t)d7w, d6 = (int16_t)d6w;
    wr32(m, F3_A5 + 0x4298u, v4298);
    wr16(m, F3_A5 + 0x42A0u, v42a0);
    int16_t d4 = 1;
    if (d7 > d6)
        d4 = -1;
    for (;;) {
        /* 0x176872 */
        uint16_t d2w = rd16(m, F3_A5 + 0x4298u);
        uint32_t d0 = divs_w(0xFFFF6000u, (uint16_t)(0x00A0u + d2w));
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 + 0x0100u);
        d2w = (uint16_t)(0x0024u - rd16(m, F3_A5 + 0x42A0u));
        uint32_t d2 = (uint32_t)((int32_t)(int16_t)d2w * (int32_t)(int16_t)(uint16_t)d0);
        d2 <<= 6;
        d2 = divs_w(d2, 0x0032u);
        d2 ^= 0x3Fu;
        uint32_t idx = (uint32_t)((int32_t)d7 * 2);
        wr16(m, a0 + idx, (uint16_t)d2);
        wr8(m, a1 + idx, (uint8_t)d0);
        d5 += d3;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d5);
        d7 = (int16_t)(d7 + d4);
        if (d4 < 0) {
            if (!(d6 <= d7)) break;
        } else {
            if (!(d7 <= d6)) break;
        }
    }
}

/* ROM 0x1768CA: line-RAM scroll ramp: low 9 bits of the words D7..D6 at A0
 * (offset by A5-0x5D56) = high word of the accumulator A5+0x4298 */
static void sub_1768CA(const f3mem_t *m, uint32_t a0, uint16_t d7w, uint16_t d6w,
                       uint32_t v4298, uint32_t d3, uint32_t d4)
{
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    int16_t d7 = (int16_t)d7w, d6 = (int16_t)d6w;
    wr32(m, F3_A5 + 0x4298u, v4298);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        wr32(m, F3_A5 + 0x4298u, 0u - rd32(m, F3_A5 + 0x4298u));
        d3 = 0u - d3;
        d4 = 0u - d4;
    }
    int16_t d5 = 1;
    if (d7 > d6)
        d5 = -1;
    for (;;) {
        /* 0x17690C: bfins D2,D0{23:9} */
        uint16_t d2w = rd16(m, F3_A5 + 0x4298u);
        uint32_t p = a0 + (uint32_t)((int32_t)d7 * 2);
        uint16_t w = rd16(m, p);
        wr16(m, p, (uint16_t)((w & 0xFE00u) | (d2w & 0x01FFu)));
        d3 += d4;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d3);
        d7 = (int16_t)(d7 + d5);
        if (d5 < 0) {
            if (!(d6 <= d7)) break;
        } else {
            if (!(d7 <= d6)) break;
        }
    }
}

/* ROM 0x0053B2: run-length tilemap blit into playfield RAM.
 * src = {cols.b, rows.b, stream...}; stream words: 0 -> next long is a tile;
 * 0x01nn -> nn+1 empty cells; 0x02nn tile -> nn+1 copies; 0x03nn tile ->
 * nn+1 cells, tile+1 each; 0x04nn tile -> tile-1 each. Tiles ^= (mode & 0xFE00)<<16.
 * mode bits 15-14 pick the scan direction (table 0x5352). The destination is a
 * 32x32-long page: inner offset wraps in 0x7C, column offset in 0xF80.
 * dry = 1 only validates the stream (returns -1 on an unknown op, no writes). */
static int sub_0053B2(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t modew, int dry)
{
    uint32_t a0 = src;
    uint16_t d7hi = (uint16_t)(rd8(m, a0) - 1u);
    uint16_t d7lo = (uint16_t)(rd8(m, a0 + 1u) - 1u);
    a0 += 2u;
    uint32_t d7 = ((uint32_t)d7hi << 16) | d7lo;
    uint16_t dst_lo = (uint16_t)dst;
    unsigned mode = (unsigned)(modew >> 14) & 3u;
    uint16_t d5hi = (mode < 2u) ? 0x0080u : 0xFF80u;
    uint16_t d5w = dst_lo;
    if (mode >= 2u)
        d5w = (uint16_t)(d5w + (uint16_t)((d7 >> 9) & 0xFFFu));
    uint16_t d6hi = dst_lo;
    if (mode & 1u)
        d6hi = (uint16_t)(d6hi + (uint16_t)(d7lo << 2));
    uint16_t d6lo = (mode & 1u) ? 0xFFFCu : 0x0004u;
    uint32_t d3 = ((uint32_t)modew & 0xFE00u) << 16;
    uint32_t a1 = dst;

    enum { FETCH, CLR, FILL, INC, DEC } st = FETCH;
    uint16_t d0 = 0u;
    uint32_t d2 = 0u;

    uint16_t outer = d7hi;
    for (;;) {
        uint16_t d4w = d6hi;
        d5w &= 0x0F80u;
        a1 = (a1 & 0xFFFFF000u) + (uint32_t)(int32_t)(int16_t)d5w;
        uint16_t inner = d7lo;
        for (;;) {
            d4w &= 0x007Cu;
            uint32_t cell = a1 + (uint32_t)(int32_t)(int16_t)d4w;
            if (st == FETCH) {
                uint16_t w = rd16(m, a0); a0 += 2u;
                if (w == 0u) {
                    d2 = rd32(m, a0) ^ d3; a0 += 4u;
                    if (!dry) wr32(m, cell, d2);
                    goto next;
                }
                d0 = (uint16_t)(w & 0xFFu);
                switch (w >> 8) {
                case 1: st = CLR; break;
                case 2: st = FILL; d2 = rd32(m, a0) ^ d3; a0 += 4u; break;
                case 3: st = INC;  d2 = rd32(m, a0) ^ d3; a0 += 4u; break;
                case 4: st = DEC;  d2 = rd32(m, a0) ^ d3; a0 += 4u; break;
                default: return -1; /* jump table 0x5438 index out of range */
                }
            }
            if (!dry) wr32(m, cell, st == CLR ? 0u : d2);
            if (st == INC) d2 = (d2 & 0xFFFF0000u) | (uint16_t)(d2 + 1u);
            if (st == DEC) d2 = (d2 & 0xFFFF0000u) | (uint16_t)(d2 - 1u);
            if (d0-- == 0u) st = FETCH;
        next:
            d4w = (uint16_t)(d4w + d6lo);
            if (inner-- == 0u) break;
        }
        d5w = (uint16_t)(d5w + d5hi);
        if (outer-- == 0u) break;
    }
    return 0;
}

/* Validate then blit (no side effects if the stream has an unknown op). */
static int blit_0053B2(const f3mem_t *m, uint32_t src, uint32_t dst, uint16_t modew)
{
    if (sub_0053B2(m, src, dst, modew, 1) < 0)
        return -1;
    return sub_0053B2(m, src, dst, modew, 0);
}

/* ROM 0x0017D6: per-channel colour fade steps (A0 = src, A1 = dst palette) */
static void sub_0017D6(const f3mem_t *m, uint32_t a0, uint32_t a1, uint32_t d6,
                       uint32_t d1, uint16_t d2w)
{
    uint32_t a2 = F3_A5 - 0x7F50u + d6;
    int32_t div = (int32_t)d1;
    uint16_t d7 = (uint16_t)(d2w - 1u);
    do {
        uint32_t s = rd32(m, a0); a0 += 4u;
        uint32_t t = rd32(m, a1); a1 += 4u;
        for (int k = 0; k < 3; k++) {
            int sh = 16 - 8 * k;                 /* bfextu {8,16,24}:8 */
            uint32_t c0 = ((s >> sh) & 0xFFu) << 8;
            uint32_t c1 = ((t >> sh) & 0xFFu) << 8;
            wr16(m, a2 + 2u * (uint32_t)k, (uint16_t)c0);
            int32_t q = (int32_t)(c1 - c0) / div; /* divsl.l */
            wr16(m, a2 + 6u + 2u * (uint32_t)k, (uint16_t)q);
        }
        a2 += 0x0Cu;
    } while (d7-- != 0u);
}

/* ROM 0x001792: allocate a palette fade slot (16 x 6 bytes at A5-0x6650) */
static void sub_001792(const f3mem_t *m, uint32_t a0, uint32_t a1,
                       uint16_t d0w, uint32_t d1, uint16_t d2w)
{
    uint32_t a3 = F3_A5 - 0x6650u;
    uint32_t d6 = 0;
    for (int d7 = 0; d7 < 16; d7++) {
        if (rd16(m, a3) == 0u) {
            wr16(m, a3 + 2u, (uint16_t)d1);
            if ((int16_t)(uint16_t)d1 <= 0) return;
            wr16(m, a3, d2w);
            if (d2w == 0u) return;
            wr16(m, a3 + 4u, d0w);
            sub_0017D6(m, a0, a1, d6, d1, d2w);
            return;
        }
        d6 += 0x180u;
        a3 += 6u;
    }
}

/* ROM 0x0018E4: C wrapper (src, colour, steps, count); A1 = 0x198E */
static void sub_0018E4(const f3mem_t *m, uint32_t src, uint32_t d0, uint32_t d1, uint32_t d2)
{
    sub_001792(m, src, 0x0000198Eu, (uint16_t)d0, d1, (uint16_t)d2);
}

/* ROM 0x0BDE92 */
int Obj_BDE92_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t v = (int16_t)rd16(m, obj + 0x60u);
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)(uint16_t)(0x20 - v) * 0x5000);
    d0 <<= 4;
    sub_176832(m, 0x0062A600u, 0x00628600u, 0x0000u, 0x00E7u, d0, 0u, 0u, 0x0003u);

    /* 0x0BDEDA: divu.w */
    uint16_t d1w = (uint16_t)(0x40 - (int16_t)rd16(m, obj + 0x60u));
    d0 = 0x00200000u;
    uint32_t q = d0 / d1w;
    if (q <= 0xFFFFu)
        d0 = ((d0 % d1w) << 16) | q;
    d0 &= 0xFFFFu;
    d0 = (uint32_t)-(int32_t)(d0 - 0xFFFFu);
    sub_1768CA(m, 0x00624600u, 0x00A0u, 0x0000u, 0u, d0, 0u);
    d0 = (uint32_t)-d0;
    sub_1768CA(m, 0x00624600u, 0x00A0u, 0x00E7u, 0u, d0, 0u);

    /* 0x0BDF58 */
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BDE92u);
        return 0;
    }

    /* 0x0BDF60 */
    if (blit_0053B2(m, 0x00090A4Au, 0x006133CCu, 0x0000u) < 0)
        return -1;
    static const uint32_t fades[14][2] = {
        {0x22040u, 0x1500u}, {0x22740u, 0x1C00u}, {0x22340u, 0x1800u},
        {0x226C0u, 0x1B80u}, {0x22640u, 0x1B00u}, {0x224C0u, 0x1980u},
        {0x220C0u, 0x1580u}, {0x219C0u, 0x0E80u}, {0x223C0u, 0x1880u},
        {0x21940u, 0x0E00u}, {0x225C0u, 0x1A80u}, {0x22240u, 0x1700u},
        {0x26DC0u, 0x1480u}, {0x26CC0u, 0x1380u},
    };
    for (int i = 0; i < 14; i++)
        sub_0018E4(m, fades[i][0], fades[i][1], 0x20u, 0x20u);

    wr16(m, obj + 0x60u, 0x003Fu);
    wr32(m, obj + 0x00u, 0x000BE16Cu);
    return 0;
}
