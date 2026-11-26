/* ROM 0x1767C8: compute vertical perspective line-scroll / zoom table.
 * Stores D0.l to A5+0x4298 (initial depth), D1.l to D5 (depth step),
 * D3.w to A5+0x42A0 (pitch/scale), and fills (A0, D7w*2) and (A1, D7w*2)
 * from row D7w to D6w (stepping +2 or -2 in word-offset units). */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_1767C8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint16_t d3w = (uint16_t)r->d[3];
    uint32_t d4 = r->d[4];
    uint32_t d5 = r->d[5];
    uint16_t d6w = (uint16_t)r->d[6];
    uint16_t d7w = (uint16_t)r->d[7];
    uint32_t a0 = r->a[0];
    uint32_t a1 = r->a[1];

    wr32(m, F3_A5 + 0x4298u, d0);                       /* 0x1767C8 */
    d5 = d1;                                            /* 0x1767CC */
    wr16(m, F3_A5 + 0x42A0u, d3w);                      /* 0x1767CE */
    uint16_t d4w = 2u;                                  /* 0x1767D2 */
    if ((int16_t)d7w > (int16_t)d6w) {                  /* 0x1767D6..0x1767D8 */
        d4w = (uint16_t)(0u - d4w);                     /* 0x1767DC */
        d6w = (uint16_t)(d6w - 1u);                     /* 0x1767DE */
    }
    d6w = (uint16_t)(d6w + d6w);                        /* 0x1767E0 */
    d7w = (uint16_t)(d7w + d7w);                        /* 0x1767E2 */

    for (;;) {
        uint16_t z = rd16(m, F3_A5 + 0x4298u);          /* 0x1767E4 */
        d2 = setw(d2, z);
        d0 = 0xFFFF6000u;                               /* 0x1767EA: #-0xa000 */
        d1 = 0x000000A0u;                               /* 0x1767F0 */
        int16_t denom = (int16_t)((uint16_t)d1 + z);    /* 0x1767F6 */
        d1 = setw(d1, (uint16_t)denom);
        if (denom != 0) {                               /* 0x1767F8: divs.w D1w, D0 */
            int32_t q = (int32_t)d0 / (int32_t)denom;
            int32_t rem = (int32_t)d0 % (int32_t)denom;
            if (q >= -32768 && q <= 32767)
                d0 = ((uint32_t)(uint16_t)(int16_t)rem << 16) | (uint16_t)(int16_t)q;
        }
        d2 = setw(d2, (uint16_t)d0);                    /* 0x1767FA */
        d0 = setw(d0, (uint16_t)((uint16_t)d0 + 0x0100u)); /* 0x1767FC */
        int16_t span = (int16_t)(0x0024u - rd16(m, F3_A5 + 0x42A0u)); /* 0x176800..0x176804 */
        d2 = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)span); /* 0x176808: muls.w */
        d2 = (uint32_t)((int32_t)d2 << 6);              /* 0x17680A: asl.l #6 */
        {                                               /* 0x17680C: divs.w #0x32, D2 */
            int32_t q = (int32_t)d2 / 0x32;
            int32_t rem = (int32_t)d2 % 0x32;
            if (q >= -32768 && q <= 32767)
                d2 = ((uint32_t)(uint16_t)(int16_t)rem << 16) | (uint16_t)(int16_t)q;
        }
        d2 = setw(d2, (uint16_t)((uint16_t)d2 ^ 0x003Fu)); /* 0x176812 */
        int32_t off = (int32_t)(int16_t)d7w;
        wr16(m, a0 + (uint32_t)off, (uint16_t)d2);      /* 0x176816 */
        wr8(m, a1 + (uint32_t)off, (uint8_t)d0);        /* 0x17681A */
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d5); /* 0x17681E */
        d7w = (uint16_t)(d7w + d4w);                    /* 0x176822 */

        uint8_t cond = ((int16_t)d7w <= (int16_t)d6w) ? 0xFFu : 0x00u; /* 0x176824..0x176826 */
        cond = (uint8_t)((cond ^ (uint8_t)d4w) & 0x0Cu); /* 0x176828..0x17682A */
        d0 = (d0 & 0xFFFFFF00u) | cond;
        if (cond == 0u)                                 /* 0x17682E */
            break;
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[4] = setw(d4, d4w);
    r->d[5] = d5;
    r->d[6] = setw(r->d[6], d6w);
    r->d[7] = setw(r->d[7], d7w);
    return 0;
}
