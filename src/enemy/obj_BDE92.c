#include "obj_BDE92.h"

/* Line RAM (0x620000..) and playfield RAM (0x610000..) are not modelled:
 * only the Work RAM side effects of the raster helpers are reproduced. */

/* ROM 0x176832: line-RAM zoom ramp; Work RAM effects only */
static void sub_176832(const f3mem_t *m, uint16_t d7w, uint16_t d6w,
                       uint32_t v4298, uint32_t d5, uint32_t d3, uint16_t v42a0)
{
    wr32(m, F3_A5 + 0x4298u, v4298);
    wr16(m, F3_A5 + 0x42A0u, v42a0);
    int16_t d7 = (int16_t)d7w, d6 = (int16_t)d6w;
    int16_t d4 = 1;
    if (d7 > d6)
        d4 = -1;
    for (;;) {
        /* 0x176872: line RAM writes skipped */
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

/* ROM 0x1768CA: line-RAM scroll ramp; Work RAM effects only */
static void sub_1768CA(const f3mem_t *m, uint16_t d7w, uint16_t d6w,
                       uint32_t v4298, uint32_t d3, uint32_t d4)
{
    wr32(m, F3_A5 + 0x4298u, v4298);
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {
        wr32(m, F3_A5 + 0x4298u, (uint32_t)-rd32(m, F3_A5 + 0x4298u));
        d3 = (uint32_t)-d3;
        d4 = (uint32_t)-d4;
    }
    int16_t d7 = (int16_t)d7w, d6 = (int16_t)d6w;
    int16_t d5 = 1;
    if (d7 > d6)
        d5 = -1;
    for (;;) {
        /* 0x17690C: line RAM bfins skipped */
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
    sub_176832(m, 0x0000u, 0x00E7u, d0, 0u, 0u, 0x0003u);

    /* 0x0BDEDA: divu.w */
    uint16_t d1w = (uint16_t)(0x40 - (int16_t)rd16(m, obj + 0x60u));
    d0 = 0x00200000u;
    uint32_t q = d0 / d1w;
    if (q <= 0xFFFFu)
        d0 = ((d0 % d1w) << 16) | q;
    d0 &= 0xFFFFu;
    d0 = (uint32_t)-(int32_t)(d0 - 0xFFFFu);
    sub_1768CA(m, 0x00A0u, 0x0000u, 0u, d0, 0u);
    d0 = (uint32_t)-d0;
    sub_1768CA(m, 0x00A0u, 0x00E7u, 0u, d0, 0u);

    /* 0x0BDF58 */
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BDE92u);
        return 0;
    }

    /* 0x0BDF60: jsr 0x53B2 only writes playfield RAM (0x6133CC) */
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
