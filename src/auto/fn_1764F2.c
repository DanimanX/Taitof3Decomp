/* ROM 0x1764F2: compute perspective row-zoom / horizontal-scale table.
 * Clears A5+0x4298 and D4, reads sin/cos at 0x27E40/0x27EC0 indexed by
 * (D2w & 0xFFFE), reverses direction when A5-0x5D58 != 0, and fills 9-bit
 * packed fields (bits 8..0) in (A0, D7w*2) from D7w to D5w. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1764F2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t d3 = r->d[3];
    uint32_t d4 = 0u;                                   /* 0x1764F6 */
    uint32_t d5 = r->d[5];
    uint16_t d6w = (uint16_t)d5;                        /* 0x1764F8 */
    uint16_t d7w = (uint16_t)r->d[7];
    uint32_t a0 = r->a[0];

    wr32(m, F3_A5 + 0x4298u, 0u);                       /* 0x1764F2 */
    d2 = setw(d2, (uint16_t)(d2 & 0xFFFEu));            /* 0x1764FA */

    int32_t ang_off = (int32_t)(int16_t)(uint16_t)d2;
    d3 = setw(d3, rd16(m, 0x00027E40u + (uint32_t)ang_off)); /* 0x17650A */
    d3 = (uint32_t)((int32_t)d3 >> 8);                  /* 0x17650E */
    d2 = setw(d2, rd16(m, 0x00027EC0u + (uint32_t)ang_off)); /* 0x176510 */

    uint16_t d5w = 1u;                                  /* 0x176514 */
    if (rd16(m, F3_A5 - 0x5D58u) != 0u) {               /* 0x176518..0x17651C */
        d2 = setw(d2, (uint16_t)(0u - (uint16_t)d2));   /* 0x176520 */
        d5w = (uint16_t)(0u - d5w);                     /* 0x176522 */
    }

    for (;;) {
        d1 = 0u;                                        /* 0x176524 */
        d0 = setw(d0, (uint16_t)d4);                    /* 0x176526 */
        if ((uint16_t)d4 != 0u) {                       /* 0x176528 */
            d1 = rd32(m, F3_A5 + 0x4298u);              /* 0x17652C */
            d1 = (uint32_t)((int32_t)d1 >> 2);          /* 0x176530 */
            d0 = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)(int16_t)d3); /* 0x176532 */
            int16_t denom = (int16_t)((uint16_t)d0 + (uint16_t)d2); /* 0x176534 */
            d0 = setw(d0, (uint16_t)denom);
            if (denom != 0) {                           /* 0x176536: divs.w D0w, D1 */
                int32_t q = (int32_t)d1 / (int32_t)denom;
                int32_t rem = (int32_t)d1 % (int32_t)denom;
                if (q >= -32768 && q <= 32767)
                    d1 = ((uint32_t)(uint16_t)(int16_t)rem << 16) | (uint16_t)(int16_t)q;
            }
            d1 = setw(d1, (uint16_t)((uint16_t)d1 - (uint16_t)d4)); /* 0x176538 */
        }

        uint32_t addr = a0 + (uint32_t)((int32_t)(int16_t)d7w * 2); /* 0x17653A */
        d0 = setw(d0, rd16(m, addr));
        d0 = (d0 & 0xFFFFFE00u) | (d1 & 0x000001FFu);   /* 0x17653E: bfins D1, D0{0x17:0x9} */
        wr16(m, addr, (uint16_t)d0);                    /* 0x176542 */

        wr16(m, F3_A5 + 0x4298u, (uint16_t)(rd16(m, F3_A5 + 0x4298u) + 1u)); /* 0x176546 */
        d4 = setw(d4, (uint16_t)((uint16_t)d4 + d5w));  /* 0x17654A */
        d7w = (uint16_t)(d7w + 1u);                     /* 0x17654C */
        if ((int16_t)d7w > (int16_t)d6w)                /* 0x17654E..0x176550 */
            break;
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[5] = setw(d5, d5w);
    r->d[6] = setw(r->d[6], d6w);
    r->d[7] = setw(r->d[7], d7w);
    r->a[3] = 0x00027E40u;
    r->a[4] = 0x00027EC0u;
    return 0;
}
