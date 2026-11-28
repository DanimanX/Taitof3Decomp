/* ROM 0x0BF622: elevator travel trigger for the player object in A4 -- when the
 * platform state is armed and the player stands between two floors, probe the
 * floor regions (0x0BD546/0x0BD516) and either install the travel ramp
 * (A5+0x1BE0 divisor, A5+0x1BDC step, A5+0x1BB0 distance) or, on a floor
 * boundary, jump into the shared timer block at 0x0BF2A6/0x0BF320. */
#include "ea_auto.h"

/* Bit n (from the MSB of the first byte) of a memory bit field, `width` bits
 * wide: the 0x0BF2CC/0x0BF2DA/0x0BF2EC bfextu reads. */
static uint32_t bfext(const f3mem_t *m, uint32_t addr, unsigned offset,
                      unsigned width)
{
    uint32_t v = 0u;

    for (unsigned i = 0; i < width; ++i) {
        unsigned bit = offset + i;
        v = (v << 1) | (uint32_t)((rd8(m, addr + (bit >> 3)) >> (7u - (bit & 7u))) & 1u);
    }
    return v;
}

/* 0x0BD516: probe the floor grid at D0.w/D1.w. Each coordinate is biased and
 * shifted, range checked, then used as a word offset into the pointer at
 * A5+0x994; the MSB of the grid byte found there lands in D0 (0x00FF when
 * set, 0x0000 when clear) and A0 = that byte. Returns the Z flag the ROM
 * leaves for the caller's 0x0BD550 bne (1 = Z set). */
static int Sub_BD516(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                     uint32_t *pd0, uint32_t *pd1, uint32_t *pa0)
{
    uint32_t d0 = *pd0, d1 = *pd1;
    uint16_t x = (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x0998u));
    int z;

    x = (uint16_t)((int16_t)x >> 2);                 /* 0x0BD51A asr.w #2 */
    d0 = set_word(d0, x);
    if (x >= rd16(m, a5 + 0x099Cu)) {                /* 0x0BD51C bcc 0x0BD542 */
        d0 = 0u;                                     /* 0x0BD542 moveq #0,d0 */
        z = 1;
    } else {
        uint16_t y = (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x099Au));

        y = (uint16_t)((int16_t)y >> 1);             /* 0x0BD526 asr.w #1 */
        d1 = set_word(d1, y);
        if (y >= rd16(m, a5 + 0x099Eu)) {            /* 0x0BD52C bcc 0x0BD542 */
            d0 = 0u;
            z = 1;
        } else {
            uint32_t a0 = rd32(m, a5 + 0x0994u);     /* 0x0BD530 */

            d0 = (uint32_t)(int32_t)(int16_t)x;      /* 0x0BD52E ext.l d0 */
            /* 0x0BD534 adda.w (a0, d1.w * 2), a0 */
            a0 += (uint32_t)(int32_t)(int16_t)
                  rd16(m, a0 + 2u * (uint32_t)(int32_t)(int16_t)y);
            /* 0x0BD538 bftst (a0){0:1} ; 0x0BD53C sne.b ; 0x0BD53E ext.w */
            d0 = set_word(d0, (rd8(m, a0) & 0x80u) != 0u ? 0x00FFu : 0x0000u);
            *pa0 = a0;
            z = (d0 & 0xFFFFu) == 0u;
        }
    }
    *pd0 = d0;
    *pd1 = d1;
    r->a[7] += 4u;                                   /* 0x0BD540 rts */
    return z;
}

/* cmp2.w (ea),Dn: 1 when the word is outside the signed {lo, hi} pair. */
static int outside_bounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);

    return (int16_t)v < lo || (int16_t)v > hi;
}

/* 0x0BD546: region lookup over the count at 0x41B818 and the record pointers
 * at 0x41B81C (each record: box 1 = +0x34..+0x3E, optional box 2 =
 * +0x24..+0x2E). D0 = 1 first box, 2 second box, 0 no hit; A0 = the record
 * (or the grid byte when 0x0BD516 short-circuits). Returns 1 when the flags
 * at its rts make the caller's `ble` branch. */
static int Sub_BD546(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                     uint32_t *pd0, uint32_t *pd1, uint32_t *pa0)
{
    uint32_t d0 = *pd0, d1 = *pd1, d2 = r->d[2];
    uint32_t a0 = *pa0, a1 = r->a[1];
    uint16_t d2w;
    int z, res = 0;

    r->a[7] -= 4u;                                   /* 0x0BD546 movem.w d0-d1,-(a7) */
    wr16(m, r->a[7] + 0u, (uint16_t)d0);
    wr16(m, r->a[7] + 2u, (uint16_t)d1);
    push_long(m, r, 0x0BD54Cu);                      /* 0x0BD54A bsr 0x0BD516 */
    z = Sub_BD516(m, r, a5, &d0, &d1, &a0);
    d0 = set_word(d0, rd16(m, r->a[7] + 0u));        /* 0x0BD54C movem.w (a7)+ */
    d1 = set_word(d1, rd16(m, r->a[7] + 2u));
    r->a[7] += 4u;
    *pd0 = d0;
    *pd1 = d1;
    *pa0 = a0;
    if (!z) {                                        /* 0x0BD550 bne 0x0BD59C */
        r->a[7] += 4u;                               /* rts */
        return 0;
    }
    if ((int16_t)rd16(m, 0x41B818u) < 0) {           /* 0x0BD552 bmi 0x0BD59A */
        *pd0 = 0u;                                   /* moveq #0,d0 */
        r->a[7] += 4u;
        return 1;
    }
    push_long(m, r, a1);                             /* 0x0BD55A movem.l d2/a1,-(a7) */
    push_long(m, r, d2);
    d2 = set_word(d2, rd16(m, 0x41B818u));           /* 0x0BD55E */
    a1 = 0x41B81Cu;                                  /* 0x0BD564 lea */
    d2w = (uint16_t)d2;
    for (;;) {
        a0 = rd32(m, a1);                            /* 0x0BD56A movea.l (a1)+,a0 */
        a1 += 4u;
        if (!outside_bounds(m, a0 + 0x0034u, (uint16_t)d0) &&
            !outside_bounds(m, a0 + 0x003Cu, (uint16_t)d1)) {
            res = 1;                                 /* 0x0BD574 bcc 0x0BD5A6 */
            break;
        }
        /* 0x0BD57C: the optional second box */
        if (rd32(m, a0 + 0x0024u) != 0u &&
            !outside_bounds(m, a0 + 0x0024u, (uint16_t)d0) &&
            !outside_bounds(m, a0 + 0x002Cu, (uint16_t)d1)) {
            res = 2;                                 /* 0x0BD58A bcc 0x0BD59E */
            break;
        }
        d2w = (uint16_t)(d2w - 1u);                  /* 0x0BD592 dbra d2 */
        if (d2w == 0xFFFFu)
            break;                                   /* 0x0BD596 */
    }
    a1 = rd32(m, r->a[7] + 4u);                      /* movem.l (a7)+,d2/a1 */
    d2 = rd32(m, r->a[7] + 0u);
    r->a[7] += 8u;
    r->d[2] = d2;
    r->a[1] = a1;
    if (res == 0) {
        *pd0 = 0u;                                   /* 0x0BD59A moveq #0,d0 */
        r->a[7] += 4u;
        return 1;
    }
    *pd0 = (uint32_t)res;                            /* 0x0BD5A2/0x0BD5AA moveq */
    r->a[7] += 4u;
    return 0;
}

/* 0x0BF670 / 0x0BF690 / 0x0BF6B0 (and the 0x0BF746 / 0x0BF766 / 0x0BF786
 * mirror): one region lookup for the object word at `off`+0xA0(A4) plus the
 * player floor word, then the two cmp.w checks against the record found.
 * Returns 1 = the +0x3C compare matched (D3 kept), 2 = any other hit (D3
 * becomes 0x72/0xAA), 0 = no record. */
static int region_probe(const f3mem_t *m, f3regs_t *r, uint32_t a5, uint32_t a4,
                        uint16_t off, uint16_t d4, uint32_t ret, uint32_t *pd0,
                        uint32_t *pd1, uint32_t *pa0)
{
    uint32_t d0 = *pd0, d1 = *pd1, a0 = *pa0;
    int hit;

    d1 = set_word(d1, rd16(m, a4 + 0x0066u));        /* 0x0BF670 */
    d0 = set_word(d0, (uint16_t)(rd16(m, a4 + off) + rd16(m, a4 + 0x00A0u)));
    push_long(m, r, ret);                            /* 0x0BF67C jsr 0x0BD546 */
    hit = !Sub_BD546(m, r, a5, &d0, &d1, &a0);       /* 0x0BF682 ble */
    *pd0 = d0;
    *pd1 = d1;
    *pa0 = a0;
    if (!hit)
        return 0;
    if (d4 == rd16(m, a0 + 0x002Cu))                 /* 0x0BF684 beq 0x0BF6D0 */
        return 2;
    if (d4 == rd16(m, a0 + 0x003Cu))                 /* 0x0BF68A beq 0x0BF6D2 */
        return 1;
    return 2;                                        /* falls into 0x0BF6D0 */
}

/* 0x0BF320: sign the distance at A5+0x1BB0 with the direction bit at A5+0x1BD4
 * and publish it as the word at A5+0x1C12. Pops its return address. */
static void Sub_BF320(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                      uint32_t *pd0, uint32_t *pd1)
{
    uint32_t d0 = *pd0, d1 = *pd1;

    d0 = set_word(d0, rd16(m, a5 + 0x1BB0u));        /* 0x0BF320 */
    d1 = set_word(d1, rd16(m, a5 + 0x1BD4u));        /* 0x0BF324 */
    if (((uint8_t)d1 & 1u) == 0u)                    /* 0x0BF328 btst.b #0,d1 */
        d0 = set_word(d0, (uint16_t)(-(int16_t)(uint16_t)d0)); /* 0x0BF32E neg.w */
    wr16(m, a5 + 0x1C12u, (uint16_t)d0);             /* 0x0BF330 */
    *pd0 = d0;
    *pd1 = d1;
    r->a[7] += 4u;                                   /* 0x0BF334 rts */
}

int Ea_0BF622(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a5 = r->a[5];
    const uint32_t a4 = r->a[4];
    const uint32_t a3 = r->a[3];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3], d4 = r->d[4];
    uint32_t a0 = r->a[0];
    int res;

    /* 0x0BF622 gating: platform armed, idle and the player between floors */
    if ((int16_t)rd16(m, a4 + 0x0086u) < 0)          /* 0x0BF622 bmi 0x0BF620 */
        goto ret;
    if (rd32(m, a5 + 0x1BDCu) != 0u)                 /* 0x0BF628 bne */
        goto ret;
    if (rd16(m, a5 + 0x1BF2u) != 0u)                 /* 0x0BF62E bne */
        goto ret;
    d0 = set_word(d0, rd16(m, a4 + 0x0068u));        /* 0x0BF634 */
    if ((uint16_t)d0 == rd16(m, a4 + 0x0066u))       /* 0x0BF638 beq */
        goto ret;
    if ((int16_t)rd16(m, a5 + 0x1BD6u) >= 0)         /* 0x0BF63E bpl */
        goto ret;
    d0 = set_word(d0, rd16(m, a5 + 0x1BD4u));        /* 0x0BF644 */
    if (((uint8_t)d0 & 1u) != 0u)                    /* 0x0BF648 bne.w 0x0BF722 */
        goto dir_b;

    /* ---- 0x0BF650: travel towards the lower floor ------------------------ */
    d2 = set_word(d2, rd16(m, a4 + 0x0066u));
    /* 0x0BF654..0x0BF666: a live object closer than the player blocks it */
    if (rd16(m, a3 + 0x0002u) != 0u && rd32(m, a3) != 0x0001FF01u &&
        (int16_t)(uint16_t)d2 > (int16_t)rd16(m, a3 + 0x0066u))
        goto ret;
    d4 = set_word(d4, (uint16_t)d2);                 /* 0x0BF668 */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BCAu)));
    d3 = 0x70u;                                      /* 0x0BF66E moveq #$70,d3 */
    res = region_probe(m, r, a5, a4, 0x00E2u, (uint16_t)d4, 0x0BF682u,
                       &d0, &d1, &a0);
    if (res == 0) {
        res = region_probe(m, r, a5, a4, 0x00DAu, (uint16_t)d4, 0x0BF6A2u,
                           &d0, &d1, &a0);
        if (res == 0)
            res = region_probe(m, r, a5, a4, 0x00DEu, (uint16_t)d4, 0x0BF6C2u,
                               &d0, &d1, &a0);
    }
    if (res == 0)                                    /* 0x0BF6C2 ble 0x0BF70C */
        goto no_stop_a;
    if (res != 1)                                    /* 0x0BF6D0 moveq #$72,d3 */
        d3 = 0x72u;
    if ((int16_t)(uint16_t)d2 <= (int16_t)(uint16_t)d3) /* 0x0BF6D4 ble */
        goto ret;
    if ((int16_t)rd16(m, a0 + 0x001Eu) <= 0)         /* 0x0BF6D8 ble */
        goto ret;
    d2 = set_word(0u, rd16(m, a0 + 0x0016u));        /* 0x0BF6E0/0x0BF6E2 */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a0 + 0x0010u)));
    goto ramp;                                       /* 0x0BF6EA beq 0x0BF70A */

no_stop_a:                                           /* 0x0BF70C */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 & 0xFFF8u));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 & 0xFFF8u));
    if ((int16_t)(uint16_t)d2 <= (int16_t)(uint16_t)d3) /* 0x0BF716 ble */
        goto ret;
    d0 = 0x38u;                                      /* 0x0BF71A moveq #$38,d0 */
    goto travel;                                     /* 0x0BF71C bra.w 0x0BF2A6 */

    /* ---- 0x0BF722: travel towards the upper floor ------------------------ */
dir_b:
    d2 = set_word(d2, rd16(m, a4 + 0x0066u));
    if (rd16(m, a3 + 0x0002u) != 0u && rd32(m, a3) != 0x0001FF01u &&
        (int16_t)(uint16_t)d2 < (int16_t)rd16(m, a3 + 0x0066u)) /* 0x0BF738 */
        goto ret;
    d4 = set_word(d4, (uint16_t)d2);                 /* 0x0BF73C */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BCAu)));
    d3 = set_word(d3, 0x00A8u);                      /* 0x0BF742 move.w #$a8,d3 */
    res = region_probe(m, r, a5, a4, 0x00E2u, (uint16_t)d4, 0x0BF758u,
                       &d0, &d1, &a0);
    if (res == 0) {
        res = region_probe(m, r, a5, a4, 0x00DAu, (uint16_t)d4, 0x0BF778u,
                           &d0, &d1, &a0);
        if (res == 0)
            res = region_probe(m, r, a5, a4, 0x00DEu, (uint16_t)d4, 0x0BF798u,
                               &d0, &d1, &a0);
    }
    if (res == 0)                                    /* 0x0BF798 ble 0x0BF7E4 */
        goto no_stop_b;
    if (res != 1)                                    /* 0x0BF7A6 move.w #$AA,d3 */
        d3 = set_word(d3, 0x00AAu);
    if ((int16_t)(uint16_t)d2 >= (int16_t)(uint16_t)d3) /* 0x0BF7AC bge */
        goto ret;
    if ((int16_t)rd16(m, a0 + 0x001Eu) >= 0)         /* 0x0BF7B4 bpl */
        goto ret;
    d2 = set_word(0u, rd16(m, a0 + 0x0010u));        /* 0x0BF7B8/0x0BF7BA */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a0 + 0x0016u)));
    goto ramp;                                       /* 0x0BF7C2 beq 0x0BF7E2 */

no_stop_b:                                           /* 0x0BF7E4 */
    if ((int16_t)(uint16_t)d2 >= (int16_t)(uint16_t)d3) /* 0x0BF7E6 bge */
        goto ret;
    d0 = 0x38u;                                      /* 0x0BF7EA moveq #$38,d0 */
    /* 0x0BF7EC bra.w 0x0BF2A6 */

travel:                                              /* 0x0BF2A6 */
    /* D0.w is the ramp length (0x38/0x70) as a 16.16 high word. */
    d0 = (d0 << 16) | (d0 >> 16);                    /* 0x0BF2A6 swap d0 */
    d0 = set_word(d0, 0u);                           /* 0x0BF2A8 clr.w d0 */
    d0 += rd32(m, a5 + 0x1BDCu);                     /* 0x0BF2AA add.l */
    d1 = rd32(m, a5 + 0x1BACu);                      /* 0x0BF2AE */
    d2 = d1 + d0;                                    /* 0x0BF2B2/0x0BF2B4 */
    d3 = set_word(rd32(m, a5 + 0x1BD2u), 0u);        /* 0x0BF2B6/0x0BF2BA */
    if ((int32_t)d2 >= (int32_t)d3) {                /* 0x0BF2BE bge 0x0BF310 */
        d0 = set_word(rd32(m, a5 + 0x1BD2u), 0u);    /* 0x0BF310/0x0BF314 */
        d0 -= rd32(m, a5 + 0x1BACu);                 /* 0x0BF316 */
        d0 -= rd32(m, a5 + 0x1BDCu);                 /* 0x0BF31A */
        goto store;                                  /* 0x0BF31E bra */
    }
    a0 = rd32(m, a5 + 0x1BB8u);                      /* 0x0BF2C0 */
    /* 0x0BF2C4..0x0BF2CC: the marker nibbles of the start and the end of the
     * step, read at A0 + (16.16 high word of the position) * 4. */
    d2 = (uint32_t)(uint16_t)(d1 >> 16) * 4u;
    d2 = bfext(m, a0 + d2, 2u, 4u);                  /* bfextu (a0){2:4},d2 */
    d3 = (uint32_t)(uint16_t)((d1 + d0) >> 16) * 4u;
    d3 = bfext(m, a0 + d3, 3u, 4u);                  /* bfextu (a0){3:4},d3 */
    if ((uint16_t)d2 == (uint16_t)d3)                /* 0x0BF2DE beq 0x0BF306 */
        goto store;
    /* 0x0BF2E2..0x0BF2F4: walk forward in 4-byte steps until the marker
     * nibble changes; D0 counts the steps. */
    d0 = 0u;                                         /* 0x0BF2E2 moveq #0,d0 */
    d1 = (uint32_t)(uint16_t)(d1 >> 16) * 4u;        /* 0x0BF2E4..0x0BF2E8 */
    for (;;) {
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 + 4u)); /* 0x0BF2EA addq.w */
        d3 = bfext(m, a0 + d1, 1u, 4u);              /* 0x0BF2EC bfextu {1:4} */
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 + 1u)); /* 0x0BF2F0 */
        if ((uint16_t)d2 != (uint16_t)d3)            /* 0x0BF2F2 beq 0x0BF2EA */
            break;
    }
    /* 0x0BF2F6..0x0BF302: the boundary position, fractional part dropped. */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + rd16(m, a5 + 0x1BACu)));
    d0 = (d0 << 16) | (d0 >> 16);                    /* 0x0BF2FA swap d0 */
    d0 = set_word(d0, 0u);                           /* 0x0BF2FC clr.w d0 */
    d0 -= rd32(m, a5 + 0x1BACu);                     /* 0x0BF2FE */
    d0 -= rd32(m, a5 + 0x1BDCu);                     /* 0x0BF302 */

store:                                               /* 0x0BF306 */
    wr32(m, a5 + 0x1BB0u, d0);
    push_long(m, r, 0x0BF30Eu);                      /* 0x0BF30A bsr 0x0BF320 */
    Sub_BF320(m, r, a5, &d0, &d1);
    goto ret;                                        /* 0x0BF30E rts */

ramp:                                                /* 0x0BF6EA / 0x0BF7C2 */
    if ((uint16_t)d2 != 0u) {
        d1 = 0x380000u;                              /* 0x0BF6EC/0x0BF7C4 */
        d0 = d1;                                     /* 0x0BF6F2/0x0BF7CA */
        d1 = d1 / (uint32_t)(uint16_t)d2;            /* 0x0BF6F4 divu.l d2,d1 */
        wr32(m, a5 + 0x1BE0u, d1);                   /* 0x0BF6F8 */
        d1 = (uint32_t)((uint64_t)d1 * (uint32_t)(uint16_t)d2); /* 0x0BF6FC mulu.l */
        wr32(m, a5 + 0x1BDCu, d1);                   /* 0x0BF700 */
        d0 -= d1;                                    /* 0x0BF704 */
        wr32(m, a5 + 0x1BB0u, d0);                   /* 0x0BF706 */
    }

ret:                                                 /* 0x0BF620 / 0x0BF70A / ... */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->a[0] = a0;
    r->a[7] += 4u;                                   /* the rts pops the PC */
    return 0;
}
