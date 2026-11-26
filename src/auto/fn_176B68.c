/* ROM 0x176B68: line-RAM gradient / perspective strip writer (A0 = dst, A1 = table, D7w = start row).
 * Advances A0 by D7w*2, then reads {count.w, val.l, step.l} segments from A1
 * until count <= 0. Negates val and step when A5-0x5D58 != 0. For each row in
 * [0..0xE7], when D7w >= 0 adds ((val << 6) >> 16) to (*(A0) ^ 0x3F) ^ 0x3F
 * and advances val += step, stopping if D7w > 0xE7. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_176B68(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1 = r->a[1];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t d3 = r->d[3];
    uint16_t d6w = (uint16_t)r->d[6];
    uint16_t d7w = (uint16_t)r->d[7];

    a0 += (uint32_t)((int32_t)(int16_t)d7w * 2);         /* 0x176B68 */

    for (;;) {
        d6w = (uint16_t)(rd16(m, a1) - 1u);              /* 0x176B6C..0x176B6E */
        a1 += 2u;
        if ((int16_t)d6w < 0)                            /* 0x176B70 */
            break;

        d0 = rd32(m, a1); a1 += 4u;                      /* 0x176B74 */
        d1 = rd32(m, a1); a1 += 4u;                      /* 0x176B76 */
        if (rd16(m, F3_A5 - 0x5D58u) != 0u) {            /* 0x176B78..0x176B7C */
            d0 = (uint32_t)(-(int32_t)d0);               /* 0x176B80 */
            d1 = (uint32_t)(-(int32_t)d1);               /* 0x176B82 */
        }

        for (;;) {
            if ((int16_t)d7w >= 0) {                     /* 0x176B84..0x176B86 */
                uint16_t w = rd16(m, a0) ^ 0x003Fu;      /* 0x176B8A..0x176B8C */
                d3 = d0 << 6;                            /* 0x176B90..0x176B92 */
                d3 = (d3 >> 16) | (d3 << 16);            /* 0x176B94 swap D3 */
                w = (uint16_t)((w + (uint16_t)d3) ^ 0x003Fu); /* 0x176B96..0x176B98 */
                d2 = setw(d2, w);
                wr16(m, a0, w);                          /* 0x176B9C */
                a0 += 2u;
                d0 += d1;                                /* 0x176B9E */
                d7w = (uint16_t)(d7w + 1u);              /* 0x176BA0 */
                if ((int16_t)d7w > 0x00E7)               /* 0x176BA2..0x176BA6 */
                    goto done;
            } else {
                a0 += 2u;                                /* 0x176BB0 */
                d7w = (uint16_t)(d7w + 1u);              /* 0x176BB4 */
            }
            d6w = (uint16_t)(d6w - 1u);                  /* 0x176BAA dbf */
            if (d6w == 0xFFFFu)
                break;
        }
    }

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[6] = setw(r->d[6], d6w);
    r->d[7] = setw(r->d[7], d7w);
    r->a[0] = a0;
    r->a[1] = a1;
    return 0;
}
