/* ROM 0x176490: line-scroll / perspective slope table writer.
 * Inputs: A0 = word buffer, D2.w = angle index, D5.w = lower index bound,
 * D7.w = upper index bound. Samples sin/cos at 0x27E40/0x27EC0 (reversed if
 * A5-0x5D58 == 0) and writes 9-bit perspective offsets into low 9 bits of
 * A0[D7..D6]. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_176490(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t d3 = r->d[3];
    uint32_t d5 = r->d[5];
    uint32_t d6 = r->d[6];
    uint32_t d7 = r->d[7];

    wr32(m, F3_A5 + 0x4298u, 0u);                       /* 0x176490 */
    uint32_t d4 = 0u;                                   /* 0x176494 */
    d6 = setw(d6, (uint16_t)d5);                        /* 0x176496 */
    d2 = setw(d2, (uint16_t)d2 & 0xFFFEu);              /* 0x176498 */
    uint32_t a3 = 0x00027E40u;                          /* 0x17649C */
    uint32_t a4 = 0x00027EC0u;                          /* 0x1764A2 */

    int32_t idx2 = (int32_t)(int16_t)(uint16_t)d2;
    d3 = setw(d3, rd16(m, a3 + (uint32_t)idx2));        /* 0x1764A8 */
    d3 = (uint32_t)((int32_t)d3 >> 8);                  /* 0x1764AC: asr.l #8,D3 */
    d2 = setw(d2, rd16(m, a4 + (uint32_t)idx2));        /* 0x1764AE */
    d5 = setw(d5, 0x0001u);                             /* 0x1764B2 */

    if (rd16(m, F3_A5 - 0x5D58u) == 0u) {               /* 0x1764B6: 0x4022A8 */
        d2 = setw(d2, (uint16_t)(0u - (uint16_t)d2));   /* 0x1764BE */
        d5 = setw(d5, (uint16_t)(0u - (uint16_t)d5));   /* 0x1764C0 */
    }

    do {                                                /* 0x1764C2 */
        d1 = 0u;                                        /* 0x1764C2 */
        d0 = setw(d0, (uint16_t)d4);                    /* 0x1764C4 */
        if ((uint16_t)d0 != 0u) {                       /* 0x1764C6 */
            d1 = rd32(m, F3_A5 + 0x4298u);              /* 0x1764CA */
            d1 = (uint32_t)((int32_t)d1 >> 2);          /* 0x1764CE */
            d0 = (uint32_t)((int32_t)(int16_t)(uint16_t)d3
                          * (int32_t)(int16_t)(uint16_t)d0); /* 0x1764D0 */
            d0 = setw(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d2)); /* 0x1764D2 */
            int16_t div = (int16_t)(uint16_t)d0;
            if (div != 0) {                             /* 0x1764D4: divs.w D0w,D1 */
                int32_t q = (int32_t)d1 / (int32_t)div;
                int32_t rem = (int32_t)d1 % (int32_t)div;
                if (q >= -32768 && q <= 32767)
                    d1 = ((uint32_t)(uint16_t)(int16_t)rem << 16)
                       | (uint16_t)(int16_t)q;
            }
            d1 = setw(d1, (uint16_t)((uint16_t)d1 - (uint16_t)d4)); /* 0x1764D6 */
        }

        uint32_t elem_addr = a0 + (uint32_t)((int32_t)(int16_t)(uint16_t)d7 * 2);
        d0 = setw(d0, rd16(m, elem_addr));              /* 0x1764D8 */
        d0 = (d0 & ~0x01FFu) | (d1 & 0x01FFu);          /* 0x1764DC: bfins D1,D0{23:9} */
        wr16(m, elem_addr, (uint16_t)d0);               /* 0x1764E0 */
        wr16(m, F3_A5 + 0x4298u, (uint16_t)(rd16(m, F3_A5 + 0x4298u) + 1u)); /* 0x1764E4 */
        d4 = setw(d4, (uint16_t)((uint16_t)d4 + (uint16_t)d5)); /* 0x1764E8 */
        d7 = setw(d7, (uint16_t)((uint16_t)d7 - 1u));   /* 0x1764EA */
    } while ((int16_t)(uint16_t)d7 >= (int16_t)(uint16_t)d6); /* 0x1764EC */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[5] = d5;
    r->d[6] = d6;
    r->d[7] = d7;
    r->a[3] = a3;
    r->a[4] = a4;
    return 0;
}
