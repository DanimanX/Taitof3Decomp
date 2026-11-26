/* ROM 0x1633CC: stage background line-scroll and palette/row-effect tick (A0 = object).
 * Applies sinusoidal horizontal line offset to 0x624600 + (A5-0x5D56) rows 0x30..0x7F
 * via 0x1769C2, applies vertical/row-scroll ramp from 0x16376E to 0x62A200 + (A5-0x5D56)
 * via 0x176B68, advances +0x44 phase by 0x11000, and re-arms 0x1633CC. */
#include "fn_auto.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x1769C2: fill rows d7w..d6w of gfx table A0 with scaled sine from +0x4298. */
static void sub_1769C2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1 = 0x00027E40u;                              /* 0x1769C2 */
    wr32(m, F3_A5 + 0x4298u, r->d[0]);                      /* 0x1769C8 */
    uint32_t d4 = r->d[1];                                  /* 0x1769CC */
    uint32_t d2 = setw(r->d[2], (uint16_t)((int16_t)r->d[2] << 2)); /* 0x1769CE */
    uint32_t d3 = 0u;                                       /* 0x1769D0 */
    a0 += (uint32_t)((int32_t)(int16_t)r->d[7] * 2);        /* 0x1769D2 */

    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d6 = r->d[6];
    uint32_t d7 = r->d[7];

    do {
        d3 = (d3 & 0xFFFFFF00u) | rd8(m, F3_A5 + 0x4299u);  /* 0x1769D6 */
        d1 = setw(d1, rd16(m, a1 + (uint32_t)((int32_t)(int16_t)d3 * 2))); /* 0x1769DA */
        d1 = (uint32_t)((int32_t)(int16_t)d2 * (int32_t)(int16_t)d1); /* 0x1769DE */
        d1 = (d1 >> 16) | (d1 << 16);                       /* 0x1769E0 */
        if ((int16_t)rd16(m, F3_A5 - 0x5D58u) != 0)         /* 0x1769E2 */
            d1 = setw(d1, (uint16_t)(-(int16_t)d1));        /* 0x1769EA */
        d0 = setw(d0, rd16(m, a0));                         /* 0x1769EC */
        d0 = (d0 & ~0x01FFu) | (d1 & 0x01FFu);              /* 0x1769EE bfins D1,D0{0x17:0x9} */
        wr16(m, a0, (uint16_t)d0);                          /* 0x1769F2 */
        a0 += 2u;
        wr32(m, F3_A5 + 0x4298u, rd32(m, F3_A5 + 0x4298u) + d4); /* 0x1769F4 */
        d7 = setw(d7, (uint16_t)((uint16_t)d7 + 1u));       /* 0x1769F8 */
    } while ((int16_t)d7 <= (int16_t)d6);                   /* 0x1769FA..0x1769FC */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[7] = d7;
    r->a[0] = a0;
    r->a[1] = a1;
}

/* ROM 0x176B68: apply piecewise-linear row scroll table A1 to gfx table A0 from D7w. */
static void sub_176B68(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a0 = r->a[0];
    uint32_t a1 = r->a[1];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t d2 = r->d[2];
    uint32_t d3 = r->d[3];
    uint32_t d6 = r->d[6];
    uint32_t d7 = r->d[7];

    a0 += (uint32_t)((int32_t)(int16_t)d7 * 2);             /* 0x176B68 */
    for (;;) {
        uint16_t cnt = (uint16_t)(rd16(m, a1) - 1u);        /* 0x176B6C..0x176B6E */
        a1 += 2u;
        d6 = setw(d6, cnt);
        if ((int16_t)cnt < 0)                               /* 0x176B70 */
            break;
        d0 = rd32(m, a1); a1 += 4u;                         /* 0x176B74 */
        d1 = rd32(m, a1); a1 += 4u;                         /* 0x176B76 */
        if ((int16_t)rd16(m, F3_A5 - 0x5D58u) != 0) {       /* 0x176B78 */
            d0 = (uint32_t)(-(int32_t)d0);                  /* 0x176B80 */
            d1 = (uint32_t)(-(int32_t)d1);                  /* 0x176B82 */
        }
        for (;;) {
            if ((int16_t)d7 < 0) {                          /* 0x176B84..0x176B86 */
                a0 += 2u;                                   /* 0x176BB0 */
                d7 = setw(d7, (uint16_t)((uint16_t)d7 + 1u)); /* 0x176BB4 */
            } else {
                d2 = setw(d2, (uint16_t)(rd16(m, a0) ^ 0x003Fu)); /* 0x176B8A..0x176B8C */
                d3 = d0 << 6;                               /* 0x176B90..0x176B92 */
                d3 = (d3 >> 16) | (d3 << 16);               /* 0x176B94 */
                d2 = setw(d2, (uint16_t)((uint16_t)d2 + (uint16_t)d3)); /* 0x176B96 */
                d2 = setw(d2, (uint16_t)((uint16_t)d2 ^ 0x003Fu)); /* 0x176B98 */
                wr16(m, a0, (uint16_t)d2);                  /* 0x176B9C */
                a0 += 2u;
                d0 += d1;                                   /* 0x176B9E */
                d7 = setw(d7, (uint16_t)((uint16_t)d7 + 1u)); /* 0x176BA0 */
                if ((int16_t)d7 > 0x00E7)                   /* 0x176BA2..0x176BA6 */
                    goto done;
            }
            cnt = (uint16_t)(cnt - 1u);                     /* 0x176BAA dbf */
            d6 = setw(d6, cnt);
            if (cnt == 0xFFFFu)
                break;
        }
    }

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[6] = d6;
    r->d[7] = d7;
    r->a[0] = a0;
    r->a[1] = a1;
}

int Fn_1633CC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];                                 /* 0x1633CC */
    int32_t scr_off = (int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);

    r->d[0] = rd32(m, obj + 0x44u);                         /* 0x1633D0..0x1633F4 */
    r->d[1] = 0x00020000u;                                  /* 0x1633E8..0x1633F2 */
    r->d[2] = setw(r->d[2], 0x0010u);                       /* 0x1633EE */
    r->d[6] = setw(r->d[6], 0x007Fu);                       /* 0x1633E2..0x1633F6 */
    r->d[7] = setw(r->d[7], 0x0030u);                       /* 0x1633DE..0x1633F8 */
    r->a[0] = (uint32_t)(0x00624600 + scr_off);             /* 0x1633D4..0x1633DA */
    sub_1769C2(m, r);                                       /* 0x1633FA */

    r->a[0] = (uint32_t)(0x0062A200 + scr_off);             /* 0x163400..0x163406 */
    r->a[1] = 0x0016376Eu;                                  /* 0x16340A */
    r->d[7] = setw(r->d[7], 0x0000u);                       /* 0x163410 */
    sub_176B68(m, r);                                       /* 0x163414 */

    r->a[0] = obj;                                          /* 0x16341A */
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 0x00011000u); /* 0x16341E */
    wr32(m, obj, 0x001633CCu);                              /* 0x163426 -> 0x1633C4 */
    return 0;
}
