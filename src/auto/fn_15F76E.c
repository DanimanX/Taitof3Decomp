/* ROM 0x15F76E: line-RAM sine-wave warp tick (A0 = object).
 * Sweeps 232 scanlines at 0x624600 + (int16_t)(A5-0x5D56) using sine table
 * 0x027E40, phase accumulator A5+0x3BBA (seeded from +0x44 << 16, step +0x1C00/
 * line starting from 0x40000), and amplitude A5+0x3BBE (0x8000, +0x300/line),
 * inserting the signed 9-bit result into the low 9 bits of each line-RAM word
 * (negated when A5-0x5D58 != 0). Advances +0x44 by 3 and re-arms 0x15F76E. */
#include "fn_auto.h"

int Fn_15F76E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = (uint32_t)rd16(m, obj + 0x44u) << 16; /* 0x15F772..0x15F778 */
    uint32_t d1 = 0x00040000u;                      /* 0x15F77A */
    uint32_t d2 = 0x00008000u;                      /* 0x15F780 */
    uint32_t tbl = 0x00027E40u;                     /* 0x15F786 */
    int16_t off = (int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t a1 = 0x0062A600u + (uint32_t)(int32_t)off; /* 0x15F78C..0x15F792 */
    uint32_t a2 = 0x00624600u + (uint32_t)(int32_t)off; /* 0x15F796..0x15F79C */

    wr32(m, F3_A5 + 0x3BBAu, d0);                   /* 0x15F7A0 */
    wr32(m, F3_A5 + 0x3BBEu, d2);                   /* 0x15F7A4 */

    uint16_t neg_flag = rd16(m, F3_A5 - 0x5D58u);
    for (int16_t d7 = 0; d7 <= 0x00E7; d7++) {      /* 0x15F7A8..0x15F7EA */
        uint32_t d3 = rd8(m, F3_A5 + 0x3BBBu);      /* 0x15F7AE */
        int16_t s = (int16_t)rd16(m, tbl + d3 * 2u); /* 0x15F7B2 */
        int16_t amp = (int16_t)rd16(m, F3_A5 + 0x3BBEu); /* 0x15F7B6 */
        uint32_t prod = (uint32_t)((int32_t)s * (int32_t)amp); /* 0x15F7BA */
        uint32_t shifted = prod << 2;               /* 0x15F7BC asl.l #2 */
        uint16_t d0w = (uint16_t)(shifted >> 16);   /* 0x15F7BE swap */
        d0 = (shifted << 16) | d0w;
        if (neg_flag != 0u) {                       /* 0x15F7C0..0x15F7C8 */
            d0w = (uint16_t)(0u - d0w);
            d0 = (d0 & 0xFFFF0000u) | d0w;
        }
        uint16_t prev = rd16(m, a2);                /* 0x15F7CA */
        uint16_t outw = (uint16_t)((prev & ~0x01FFu) | (d0w & 0x01FFu)); /* 0x15F7CC bfins {23:9} */
        wr16(m, a2, outw);                          /* 0x15F7D0 */
        a2 += 2u;
        wr32(m, F3_A5 + 0x3BBAu, rd32(m, F3_A5 + 0x3BBAu) + d1); /* 0x15F7D2 */
        d1 += 0x00001C00u;                          /* 0x15F7D6 */
        wr32(m, F3_A5 + 0x3BBEu, rd32(m, F3_A5 + 0x3BBEu) + 0x00000300u); /* 0x15F7DC */
    }

    wr16(m, obj + 0x44u, (uint16_t)(rd16(m, obj + 0x44u) + 3u)); /* 0x15F7F0 */
    wr32(m, obj, 0x0015F76Eu);                      /* 0x15F7F4 -> 0x15F766 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
