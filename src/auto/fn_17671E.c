/* ROM 0x17671E: write symmetric 9-bit ramp values into line table A0.
 * Scans downward D6=D7..0 adding D3 to 16.16 accumulator D2, then scans
 * upward D7+1..0xFF subtracting D3 from D2; negates when (A5-0x5D58).w != 0
 * and inserts bits [8:0] into each word at A0[i*2]. */
#include "fn_auto.h"

int Fn_17671E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d3 = r->d[3];
    uint16_t d7w = (uint16_t)r->d[7];
    uint16_t d6w = d7w;                                   /* 0x17671E */
    uint32_t d2 = 0u;                                     /* 0x176720 */
    uint16_t d4w = (uint16_t)r->d[4];

    for (;;) {
        uint16_t d0w = (uint16_t)(d6w + d6w);             /* 0x176722..0x176724 */
        d0 = (d0 & 0xFFFF0000u) | d0w;
        d1 = (d2 >> 16) | (d2 << 16);                     /* 0x176726..0x176728 */
        if (rd16(m, F3_A5 - 0x5D58u) != 0u)               /* 0x17672A */
            d1 = (d1 & 0xFFFF0000u) | (uint16_t)(0u - (uint16_t)d1);
        uint32_t addr = (uint32_t)((int32_t)a0 + (int32_t)(int16_t)d0w);
        d4w = (uint16_t)(rd16(m, addr) & 0xFE00u);        /* 0x176734..0x176738 */
        d1 = (d1 & 0xFFFF0000u) | ((uint16_t)d1 & 0x01FFu); /* 0x17673C */
        d4w = (uint16_t)(d4w | (uint16_t)d1);             /* 0x176740 */
        wr16(m, addr, d4w);                               /* 0x176742 */
        d2 += d3;                                         /* 0x176746 */
        d6w = (uint16_t)(d6w - 1u);
        if (d6w == 0xFFFFu)                               /* 0x176748 dbf */
            break;
    }

    d7w = (uint16_t)(d7w + 1u);                           /* 0x17674C */
    d2 = 0u;                                              /* 0x17674E */

    for (;;) {
        d1 = (d2 >> 16) | (d2 << 16);                     /* 0x176750..0x176752 */
        if (rd16(m, F3_A5 - 0x5D58u) != 0u)               /* 0x176754 */
            d1 = (d1 & 0xFFFF0000u) | (uint16_t)(0u - (uint16_t)d1);
        uint32_t addr = (uint32_t)((int32_t)a0 + (int32_t)(int16_t)d7w * 2);
        d4w = (uint16_t)(rd16(m, addr) & 0xFE00u);        /* 0x17675E..0x176762 */
        d1 = (d1 & 0xFFFF0000u) | ((uint16_t)d1 & 0x01FFu); /* 0x176766 */
        d4w = (uint16_t)(d4w | (uint16_t)d1);             /* 0x17676A */
        wr16(m, addr, d4w);                               /* 0x17676C */
        d2 -= d3;                                         /* 0x176770 */
        d7w = (uint16_t)(d7w + 1u);                       /* 0x176772 */
        if ((int16_t)d7w >= 0x0100)                       /* 0x176774..0x176778 */
            break;
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[4] = (r->d[4] & 0xFFFF0000u) | d4w;
    r->d[6] = (r->d[6] & 0xFFFF0000u) | d6w;
    r->d[7] = (r->d[7] & 0xFFFF0000u) | d7w;
    return 0;
}
