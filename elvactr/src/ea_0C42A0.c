/* ROM 0x0C42A0: rider step for the elevator platform object. Four integration
 * passes advance the 16.16 position at A2+8/+0xC by the deltas at A2+0x10/
 * +0x12, probing the floor grid (0x0BD546) after each one; a probe hit hands
 * over to the hand-off code at 0x0C431E (first box) or 0x0C4352 (second box),
 * which emits the step sprite commands and re-arms the frame script. No hit
 * publishes the floor-relative sprite command and arms 0x0C426A. */
#include "ea_auto.h"

/* 0x0BD516: probe the floor grid at D0.w/D1.w. Both coordinates are biased
 * and shifted, range checked against the A5 grid limits, then used to walk the
 * per-row offset table at A5+0x994; the row is a bit map whose column is the
 * bit offset (0x0BD538 bftst (a0){D0:2}). Returns 1 when the Z flag it leaves
 * makes the caller's 0x0BD550 bne fall through (grid cell free). */
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
            int32_t off = (int32_t)(int16_t)x;
            uint32_t bits = 0u;

            /* 0x0BD534 adda.w (a0, d1.w * 2), a0 */
            a0 += (uint32_t)(int32_t)(int16_t)
                  rd16(m, a0 + 2u * (uint32_t)(int32_t)(int16_t)y);
            /* 0x0BD538 bftst (a0){D0:1}: the extension word 0x0801 has DOFF
             * set (bit 11) with D0 as the offset and 1 as the width, so the
             * row's column is the bit offset. 0x0BD53C sne.b ; 0x0BD53E ext.w. */
            for (int i = 0; i < 1; ++i) {
                int32_t bit = off + i;

                bits |= (uint32_t)((rd8(m, a0 + (uint32_t)(bit >> 3)) >>
                                    (7u - (unsigned)(bit & 7))) & 1u);
            }
            d0 = set_word(d0, bits != 0u ? 0xFFFFu : 0x0000u);
            *pa0 = a0;
            z = bits == 0u;
        }
    }
    *pd0 = d0;
    *pd1 = d1;
    r->a[7] += 4u;                                   /* 0x0BD540 rts */
    return z;
}

/* cmp2.w (ea),Dn: 1 when the word is outside the signed {lo, hi} pair. */
static int OutsideBounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);

    return (int16_t)v < lo || (int16_t)v > hi;
}

/* 0x0BD546: region lookup over the count at 0x41B818 and the record pointers
 * at 0x41B81C (each record: box 1 = +0x34..+0x3E, optional box 2 =
 * +0x24..+0x2E). D0 = 1 first box, 2 second box, 0 no hit; A0 = the record
 * (or the row pointer when 0x0BD516 short-circuits).
 * Returns the flags it leaves for the caller's bgt/bne:
 *   0 = no record (moveq #0,d0: Z set),
 *   1 = record found (moveq #1/#2,d0: Z clear, N clear),
 *   2 = grid bit set (0x0BD516's ext.w left 0xFFFF: Z clear, N set). */
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
    if (!z) {                                        /* 0x0BD550 bne 0x0BD59C */
        *pa0 = a0;
        r->a[7] += 4u;                               /* rts */
        return 2;
    }
    *pa0 = a0;
    if ((int16_t)rd16(m, 0x41B818u) < 0) {           /* 0x0BD552 bmi 0x0BD59A */
        *pd0 = 0u;                                   /* moveq #0,d0 */
        r->a[7] += 4u;
        return 0;
    }
    push_long(m, r, a1);                             /* 0x0BD55A movem.l d2/a1,-(a7) */
    push_long(m, r, d2);
    d2 = set_word(d2, rd16(m, 0x41B818u));           /* 0x0BD55E */
    a1 = 0x41B81Cu;                                  /* 0x0BD564 lea */
    d2w = (uint16_t)d2;
    for (;;) {
        a0 = rd32(m, a1);                            /* 0x0BD56A movea.l (a1)+,a0 */
        a1 += 4u;
        if (!OutsideBounds(m, a0 + 0x0034u, (uint16_t)d0) &&
            !OutsideBounds(m, a0 + 0x003Cu, (uint16_t)d1)) {
            res = 1;                                 /* 0x0BD574 bcc 0x0BD5A6 */
            break;
        }
        /* 0x0BD57C: the optional second box */
        if (rd32(m, a0 + 0x0024u) != 0u &&
            !OutsideBounds(m, a0 + 0x0024u, (uint16_t)d0) &&
            !OutsideBounds(m, a0 + 0x002Cu, (uint16_t)d1)) {
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
    *pa0 = a0;                                       /* the last record scanned */
    if (res == 0) {
        *pd0 = 0u;                                   /* 0x0BD59A moveq #0,d0 */
        r->a[7] += 4u;
        return 0;
    }
    *pd0 = (uint32_t)res;                            /* 0x0BD5A2/0x0BD5AA moveq */
    r->a[7] += 4u;
    return 1;
}

/* 0x0C412E: unlink the object in A2 from the A4 table (count at A4+0x108,
 * pointers from A4+0x10C). D0 and A0 are scratch for the caller. */
static void Sub_C412E(const f3mem_t *m, f3regs_t *r, uint32_t a4, uint32_t a2)
{
    uint32_t a0 = a4 + 0x010Cu;                      /* 0x0C412E lea 0x10c(a4),a0 */
    uint32_t d0 = set_word(r->d[0], rd16(m, a4 + 0x0108u)); /* 0x0C4132 */
    uint16_t n = (uint16_t)d0;

    for (;;) {
        uint32_t v = rd32(m, a0);                    /* 0x0C4136 cmpa.l (a0)+,a2 */

        a0 += 4u;
        if (v == a2) {
            wr32(m, a0 - 4u, 0u);                    /* 0x0C413A clr.l -4(a0) */
            break;
        }
        n = (uint16_t)(n - 1u);                      /* 0x0C4140 dbra d0 */
        if (n == 0xFFFFu)
            break;
    }
    r->d[0] = set_word(d0, n);
    r->a[0] = a0;
}

/* 0x0C416C: walk the object list at A1 (D0.w+1 entries), clearing every entry
 * whose object sits within {-4,4} of the reference pair in D4/D5. */
static void Sub_C416C(const f3mem_t *m, f3regs_t *r, uint32_t a1, uint16_t count)
{
    const uint32_t a2 = r->a[2];
    const uint16_t d4w = (uint16_t)r->d[4], d5w = (uint16_t)r->d[5];
    uint32_t a0 = r->a[0], d2 = r->d[2], d3 = r->d[3];
    uint16_t n = count;

    for (;;) {
        d2 = rd32(m, a1);                            /* 0x0C416C move.l (a1)+,d2 */
        a1 += 4u;
        if ((int32_t)d2 > 0) {                       /* 0x0C416E ble 0x0C41A2 */
            a0 = d2;
            if (rd8(m, a0 + 0x001Eu) == 0u &&        /* 0x0C4172 tst.b 0x1e */
                a0 != a2) {                          /* 0x0C4178 beq 0x0C41A2 */
                d2 = set_word(d2, rd16(m, a0 + 0x0008u)); /* 0x0C417C */
                d3 = set_word(d3, rd16(m, a0 + 0x000Cu)); /* 0x0C4180 */
                d2 = set_word(d2, (uint16_t)((uint16_t)d2 - d4w)); /* 0x0C4184 */
                d3 = set_word(d3, (uint16_t)((uint16_t)d3 - d5w)); /* 0x0C4186 */
                /* 0x0C4188/0x0C4190 cmp2.w {-4,4},Dn ; bcs 0x0C41A2 */
                if (!OutsideBounds(m, 0x0C41A8u, (uint16_t)d2) &&
                    !OutsideBounds(m, 0x0C41A8u, (uint16_t)d3)) {
                    wr32(m, a1 - 4u, 0u);            /* 0x0C4198 clr.l -4(a1) */
                    wr32(m, a0 + 0x0004u, 0u);       /* 0x0C419C clr.l 4(a0) */
                    wr16(m, a0, 0u);                 /* 0x0C41A0 clr.w (a0) */
                }
            }
        }
        n = (uint16_t)(n - 1u);                      /* 0x0C41A2 dbra d0 */
        if (n == 0xFFFFu)
            break;                                   /* 0x0C41A6 rts */
    }
    r->a[0] = a0;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[1] = a1;
    r->d[0] = set_word(r->d[0], n);
}

/* 0x0C4146: unlink every object too close to the object in A2, from the two
 * lists at A5-0x6F2 and A5+0x1EA (a negative count skips a list). */
static void Sub_C4146(const f3mem_t *m, f3regs_t *r, uint32_t a5)
{
    const uint32_t a2 = r->a[2];
    uint32_t d0;

    r->d[4] = set_word(r->d[4], rd16(m, a2 + 0x0008u)); /* 0x0C4146 */
    r->d[5] = set_word(r->d[5], rd16(m, a2 + 0x000Cu)); /* 0x0C414A */
    d0 = set_word(r->d[0], rd16(m, a5 - 0x06F6u));   /* 0x0C414E */
    r->d[0] = d0;
    if ((int16_t)(uint16_t)d0 >= 0) {                /* 0x0C4152 bmi 0x0C415C */
        push_long(m, r, 0x0C415Cu);                  /* 0x0C4158 bsr 0x0C416C */
        Sub_C416C(m, r, a5 - 0x06F2u, (uint16_t)d0);
        r->a[7] += 4u;
    }
    d0 = set_word(r->d[0], rd16(m, a5 + 0x01E6u));   /* 0x0C415C */
    r->d[0] = d0;
    if ((int16_t)(uint16_t)d0 >= 0) {                /* 0x0C4160 bmi 0x0C416A */
        push_long(m, r, 0x0C416Au);                  /* 0x0C4166 bsr 0x0C416C */
        Sub_C416C(m, r, a5 + 0x01EAu, (uint16_t)d0);
        r->a[7] += 4u;
    }
}

/* 0x0BDE8C: tile type of the 4-bit map at A5+0x9A0 for column D0.w / row
 * D1.w (both floored to the map cell). D0 = 0 when the cell is off the map. */
static uint32_t Sub_BDE8C(const f3mem_t *m, uint32_t a5, uint32_t d0_in,
                          uint32_t d1_in, uint32_t *pa0)
{
    uint16_t d1 = (uint16_t)((uint16_t)d1_in - rd16(m, a5 + 0x09A6u));
    uint16_t d0;
    uint32_t a0;
    uint32_t off;

    d1 = (uint16_t)((int16_t)d1 >> 3);               /* 0x0BDE90 asr.w #3 */
    if (d1 >= rd16(m, a5 + 0x09AAu))                 /* 0x0BDE96 bcc 0x0BDEB4 */
        return 0u;                                   /* 0x0BDEB4 moveq #0,d0 */
    d0 = (uint16_t)((uint16_t)d0_in - rd16(m, a5 + 0x09A4u));
    d0 = (uint16_t)((int16_t)d0 >> 3);               /* 0x0BDE9C asr.w #3 */
    if (d0 >= rd16(m, a5 + 0x09A8u))                 /* 0x0BDEA2 bcc 0x0BDEB4 */
        return 0u;
    off = (uint32_t)(int32_t)(int16_t)d0 * 4u;       /* 0x0BDEA4 ext.l ; lsl.l #2 */
    /* 0x0BDEA8 movea.l ([$9a0,a5],d1.w*4),a0: post-indexed indirect -- the row
     * base comes from the long at A5+0x9A0, then the row index * 4. */
    a0 = rd32(m, a5 + 0x09A0u) + (uint32_t)(int32_t)(int16_t)d1 * 4u;
    *pa0 = a0;
    /* 0x0BDEAE bfextu (a0){D0:4},d0 */
    {
        uint32_t v = 0u;

        for (int i = 0; i < 4; ++i) {
            int32_t bit = (int32_t)off + i;

            v |= (uint32_t)((rd8(m, a0 + (uint32_t)(bit >> 3)) >>
                             (7u - (unsigned)(bit & 7))) & 1u);
        }
        return v;
    }
}

/* 0x003706: scale the frame counter at A5-0x7F2A by the rate at A5-0x7E1A and
 * publish the low 7 bits at A5-0x7E18. */
static void Sub_3706(const f3mem_t *m, f3regs_t *r, uint32_t a5)
{
    uint32_t d0 = set_word(r->d[0], 0u);             /* 0x00370A moveq #0,d0 */

    d0 = set_word(d0, rd16(m, a5 - 0x7F2Au));        /* 0x00370C */
    if (rd16(m, a5 - 0x7E1Cu) != 0u) {               /* 0x003710 beq 0x003728 */
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 0x20u)); /* 0x003716 */
        if ((int16_t)(uint16_t)d0 < 0) {             /* 0x00371A bge 0x00373C */
            int16_t p = (int16_t)((uint16_t)d0 *
                                  (uint16_t)rd16(m, a5 - 0x7E1Au)); /* muls.w */

            d0 = set_word(d0, (uint16_t)((uint16_t)p >> 8));  /* 0x003720 asr.w #8 */
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 +
                                         rd16(m, a5 - 0x7E1Au))); /* 0x003722 */
        } else {
            d0 = set_word(d0, rd16(m, a5 - 0x7E1Au)); /* 0x00373C */
        }
    } else {
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 0x20u)); /* 0x003728 */
        if ((int16_t)(uint16_t)d0 > 0) {             /* 0x00372C ble 0x00373C */
            int16_t p = (int16_t)((uint16_t)d0 *
                                  (uint16_t)rd16(m, a5 - 0x7E1Au));

            d0 = set_word(d0, (uint16_t)((uint16_t)p >> 8));  /* 0x003732 */
            d0 = set_word(d0, (uint16_t)(0u - (uint16_t)d0)); /* 0x003734 neg.w */
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 +
                                         rd16(m, a5 - 0x7E1Au))); /* 0x003736 */
        } else {
            d0 = set_word(d0, rd16(m, a5 - 0x7E1Au)); /* 0x00373C */
        }
    }
    wr8(m, a5 - 0x7E18u, (uint8_t)((uint8_t)d0 & 0x7Fu)); /* 0x003740 andi.b #$7f */
    r->d[0] = d0;
}

/* 0x0034A0: sound command 0x58B for the sample the sprite definition indexed
 * by the word argument (D2) selects. Register transparent. */
static void Sub_34A0(const f3mem_t *m, f3regs_t *r, uint32_t a5,
                     uint16_t arg_d2, uint16_t arg_d3)
{
    const uint32_t d0_save = r->d[0], d1_save = r->d[1];
    const uint32_t a0_save = r->a[0], a1_save = r->a[1], a2_save = r->a[2];
    const uint32_t tbl = rd32(m, a5 - 0x7EB4u);      /* 0x0034A8 */
    uint32_t a0 = rd32(m, tbl + 4u * (uint32_t)(int32_t)(int16_t)arg_d2);
    uint32_t d0, a1;
    uint8_t v;

    a0 += 1u;                                        /* 0x0034B0 adda.w #1 */
    d0 = rd8(m, a0);                                 /* 0x0034B6 move.b (a0),d0 */
    d0 = (uint16_t)((uint16_t)d0 - 1u);              /* 0x0034B8 subi.w #1 */
    d0 = (uint16_t)((uint16_t)d0 << 3);              /* 0x0034BC asl.w #3 */
    d0 = (uint16_t)((uint16_t)d0 + rd8(m, a0 + 2u)); /* 0x0034BE add.b 2(a0),d0 */
    v = rd8(m, a5 - 0x6171u + (uint32_t)(uint16_t)d0); /* 0x0034C2 */
    if (v == 0u)                                     /* 0x0034C8 beq 0x0034F8 */
        goto out;
    if (v != rd8(m, a0 + 5u))                        /* 0x0034CA cmp.b 5(a0) */
        goto out;

    a1 = 0xC00000u;                                  /* 0x0034D0 lea */
    r->d[1] = set_word(r->d[1],                      /* 0x0034D6/0x0034DA */
                       (uint16_t)(rd16(m, a1 + 0x480u) >> 1));
    {
        uint32_t p = a5 - 0x7E2Cu;                   /* 0x0034DC lea -0x7e2c(a5),a2 */

        wr16(m, p, 0x058Bu);                         /* 0x0034E0 */
        wr8(m, p + 2u, rd8(m, a0));                  /* 0x0034E4 */
        wr8(m, p + 3u, rd8(m, a0 + 2u));             /* 0x0034E6 */
        wr8(m, p + 4u, (uint8_t)(arg_d3 & 0xFFu));   /* 0x0034EA move.b $1b(a7) */
    }
    r->a[1] = a1;
    push_long(m, r, 0x0034F2u);                      /* 0x0034EE bsr 0x002E8C */
    Ea_002E8C(m, r);
    wr16(m, a1 + 0x480u,                            /* 0x0034F2/0x0034F4 */
         (uint16_t)((uint16_t)r->d[1] + (uint16_t)r->d[1]));
out:
    r->d[0] = d0_save;                               /* 0x0034F8 movem.l */
    r->d[1] = d1_save;
    r->a[0] = a0_save;
    r->a[1] = a1_save;
    r->a[2] = a2_save;
}

/* The 0x002E8C calls of the 0x002F3A handlers: A1 = the DPRAM base, D1 = the
 * write index, popped return address, result = the advanced index. */
static uint32_t SendCmd(const f3mem_t *m, f3regs_t *r, uint32_t a1,
                        uint16_t d1w, uint32_t ret)
{
    r->a[1] = a1;
    r->d[1] = set_word(r->d[1], d1w);
    push_long(m, r, ret);
    Ea_002E8C(m, r);
    return (uint16_t)r->d[1];
}

/* 0x002F3A: dispatch the sprite definition indexed by the word argument (D2)
 * through the handler table at 0x2F84. Register transparent. */
static void Sub_2F3A(const f3mem_t *m, f3regs_t *r, uint32_t a5, uint16_t arg)
{
    const uint32_t d0_save = r->d[0], d1_save = r->d[1], d2_save = r->d[2];
    const uint32_t a0_save = r->a[0], a1_save = r->a[1], a2_save = r->a[2];
    uint32_t a1, a0, d1;
    uint16_t sel;

    if (rd8(m, a5 - 0x7EB0u) != 0u)                  /* 0x002F3E bne 0x002F7E */
        goto out;
    if ((int16_t)arg < 0)                            /* 0x002F48 bmi */
        goto out;
    if ((int16_t)arg > (int16_t)rd16(m, 0x00008334u)) /* 0x002F4A..0x002F52 */
        goto out;
    a1 = 0xC00000u;                                  /* 0x002F54 */
    d1 = (uint16_t)(rd16(m, a1 + 0x480u) >> 1);      /* 0x002F5A/0x002F5E */
    a0 = rd32(m, a5 - 0x7EB4u);                      /* 0x002F60 */
    a0 = rd32(m, a0 + 4u * (uint32_t)(int16_t)arg);  /* 0x002F64 */
    {
        uint32_t idx = rd8(m, a0);                   /* 0x002F6A move.b (a0)+ */

        a0 += 1u;
        sel = rd16(m, 0x00002F84u + 2u * idx);       /* 0x002F6C table */
    }
    switch (sel) {
    case 0x000Au:                                    /* 0x002F8E */
        if ((int8_t)rd8(m, a0) < 0) {                /* 0x002F92 bmi 0x002FD0 */
            uint32_t d2 = (uint16_t)(rd8(m, a0) & 0x7Fu);

            wr8(m, a5 - 0x7EACu + d2, rd8(m, a0));   /* 0x002FD8 */
            wr16(m, a5 - 0x7E2Cu, 0x0381u);          /* 0x002FE2 */
            wr8(m, a5 - 0x7E2Au, rd8(m, a0));        /* 0x002FE6 */
            d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x002FE8);

            wr16(m, a5 - 0x7E1Cu, 0u);               /* 0x002FEC */
            wr16(m, a5 - 0x7E1Au, 0u);               /* 0x002FF0 */
            wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u));   /* 0x002FF4 */
            Sub_3706(m, r, a5);                      /* 0x002FFA */
            wr16(m, a5 - 0x7E2Cu, 0x0486u);          /* 0x003002 */
            wr8(m, a5 - 0x7E2Au, rd8(m, a0));        /* 0x003006 */
            wr8(m, a5 - 0x7E29u, rd8(m, a5 - 0x7E18u)); /* 0x003008 */
            d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x00300C);

            wr16(m, a5 - 0x7E2Cu, 0x0486u);          /* 0x003010 */
            wr8(m, a5 - 0x7E2Au, rd8(m, a0));        /* 0x003014 */
            wr8(m, a5 - 0x7E29u, (uint8_t)d2);       /* 0x003018 move.b d2 */
            wr8(m, a5 - 0x7E28u, rd8(m, a5 - 0x7E18u)); /* 0x00301A */
            d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x003022);

        } else {
            uint32_t d0 = rd8(m, a0);

            wr8(m, a5 - 0x7EACu + d0, rd8(m, a0));   /* 0x002F98 */
            wr16(m, a5 - 0x7E2Cu, 0x0381u);          /* 0x002FA0 */
            wr8(m, a5 - 0x7E2Au, rd8(m, a0));        /* 0x002FA4 */
            d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x002FA6);

            wr16(m, a5 - 0x7E1Cu, 0u);               /* 0x002FAA */
            wr16(m, a5 - 0x7E1Au, 0u);               /* 0x002FAE */
            wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u));   /* 0x002FB2 */
            Sub_3706(m, r, a5);                      /* 0x002FB8 */
            wr16(m, a5 - 0x7E2Cu, 0x0486u);          /* 0x002FC0 */
            wr8(m, a5 - 0x7E2Au, rd8(m, a0));        /* 0x002FC4 */
            wr8(m, a5 - 0x7E29u, rd8(m, a5 - 0x7E18u)); /* 0x002FC6 */
            d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x002FCE);

        }
        break;
    case 0x00A0u: {                                  /* 0x003024 */
        uint32_t d0 = (uint16_t)(rd8(m, a0) & 0x7Fu);

        wr8(m, a5 - 0x7EACu + d0, rd8(m, a0));       /* 0x003030 */
        wr16(m, a5 - 0x7E2Cu, 0x0381u);              /* 0x003038 */
        wr8(m, a5 - 0x7E2Au, rd8(m, a0));            /* 0x00303C */
        d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x00303E);
        wr16(m, a5 - 0x7E1Cu, 0u);                   /* 0x003042 */
        wr16(m, a5 - 0x7E1Au, 0u);                   /* 0x003046 */
        wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u));       /* 0x00304A */
        Sub_3706(m, r, a5);                          /* 0x003050 */
        wr16(m, a5 - 0x7E2Cu, 0x0486u);              /* 0x003058 */
        wr8(m, a5 - 0x7E2Au, rd8(m, a0));            /* 0x00305C */
        wr8(m, a5 - 0x7E29u, rd8(m, a5 - 0x7E18u));  /* 0x00305E */
        d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x003062);
        wr16(m, a5 - 0x7E2Cu, 0x0488u);              /* 0x00306A */
        wr8(m, a5 - 0x7E2Au, rd8(m, a0));            /* 0x00306E */
        wr8(m, a5 - 0x7E29u, rd8(m, a0 + 2u));       /* 0x003070 */
        d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x003078);
        break;
    }
    case 0x00F6u: {                                  /* 0x00307A */
        uint32_t d0 = (uint16_t)((uint16_t)rd8(m, a0) - 1u);
        uint32_t p2;

        d0 = (uint16_t)((uint16_t)d0 << 3);          /* 0x003082 asl.w #3 */
        d0 = (uint16_t)((uint16_t)d0 + rd8(m, a0 + 2u)); /* 0x003084 */
        /* 0x003088 pea 0x9eae(a5,d0.w*2) ; 0x00308E pea 0x9e8f(a5,d0.w) */
        push_long(m, r, a5 - 0x6152u + (uint32_t)(uint16_t)d0 * 2u);
        push_long(m, r, a5 - 0x6171u + (uint32_t)(uint16_t)d0);
        if (rd16(m, a5 - 0x6152u + (uint32_t)(uint16_t)d0 * 2u) != 0u &&
            rd8(m, a5 - 0x6171u + (uint32_t)(uint16_t)d0) != 0u) {
            uint8_t v = rd8(m, a5 - 0x6171u + (uint32_t)(uint16_t)d0);

            wr16(m, a5 - 0x7E2Cu, 0x058Fu);          /* 0x0030A8 */
            wr8(m, a5 - 0x7E2Au, rd8(m, a0));        /* 0x0030AC */
            wr8(m, a5 - 0x7E29u, rd8(m, a0 + 2u));   /* 0x0030AE */
            wr8(m, a5 - 0x7E28u, v);                 /* 0x0030B2 */
            d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x0030B4);
        }
        wr16(m, a5 - 0x7E2Cu, 0x068Du);              /* 0x0030BC */
        wr8(m, a5 - 0x7E2Au, rd8(m, a0));            /* 0x0030C0 */
        wr8(m, a5 - 0x7E29u, rd8(m, a0 + 2u));       /* 0x0030C2 */
        wr8(m, a5 - 0x7E28u, rd8(m, a0 + 3u));       /* 0x0030C6 */
        wr8(m, a5 - 0x7E27u, rd8(m, a0 + 4u));       /* 0x0030CA */
        d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x0030CE);
        wr16(m, a5 - 0x7E1Cu, 0xFFFFu);              /* 0x0030D2 */
        wr16(m, a5 - 0x7E1Au, 0u);                   /* 0x0030D8 */
        wr8(m, a5 - 0x7E19u, rd8(m, a0 + 1u));       /* 0x0030DC */
        Sub_3706(m, r, a5);                          /* 0x0030E2 */
        wr16(m, a5 - 0x7E2Cu, 0x068Eu);              /* 0x0030EA */
        wr8(m, a5 - 0x7E2Au, rd8(m, a0));            /* 0x0030EE */
        wr8(m, a5 - 0x7E29u, rd8(m, a0 + 2u));       /* 0x0030F0 */
        wr8(m, a5 - 0x7E28u, rd8(m, a0 + 5u));       /* 0x0030F4 */
        wr8(m, a5 - 0x7E27u, rd8(m, a5 - 0x7E18u));  /* 0x0030F8 */
        d1 = SendCmd(m, r, a1, (uint16_t)d1, 0x0030FC);
        p2 = rd32(m, r->a[7]);                       /* 0x003100 movea.l (a7)+,a2 */
        r->a[7] += 4u;
        wr8(m, p2, rd8(m, a0 + 5u));                 /* 0x003102 */
        p2 = rd32(m, r->a[7]);                       /* 0x003106 movea.l (a7)+,a2 */
        r->a[7] += 4u;
        d0 = set_word(0u, rd8(m, a0 + 6u));          /* 0x003108 moveq #0,d0 */
        d0 = set_word(d0, (uint16_t)(int16_t)(int8_t)rd8(m, a0 + 6u)); /* 0x00310E ext.w */
        if ((int16_t)(uint16_t)d0 < 0) {             /* 0x003110 bmi 0x00311A */
            /* keep d0 */
        } else if ((uint16_t)d0 != 0u) {             /* 0x003112 bne 0x003116 */
            d0 = set_word(d0, (uint16_t)(int16_t)
                          ((int16_t)(uint16_t)d0 * 0x003C));
        } else {
            d0 = set_word(d0, 0x0001u);              /* 0x003114 moveq #1,d0 */
            d0 = set_word(d0, (uint16_t)(int16_t)(0x003C));
        }
        wr16(m, p2, (uint16_t)d0);                   /* 0x00311A */
        break;
    }
    case 0x019Au: {                                  /* 0x00311E */
        uint32_t d0 = rd8(m, a0);                    /* moveq #0,d0 ; move.b (a0)+,d0 */

        a0 += 1u;
        for (;;) {
            wr8(m, a1 + (d1 & 0x3FFu), rd8(m, a0));  /* 0x003124 */
            a0 += 1u;
            d1 = (uint16_t)((d1 + 1u) & 0x3FFu);     /* 0x003128/0x00312A */
            if ((uint16_t)d0 == 0u)                  /* 0x00312E dbra d0 */
                break;
            d0 = (uint16_t)((uint16_t)d0 - 1u);
        }
        break;
    }
    case 0x01B0u:                                    /* 0x003134: bare rts */
    default:
        break;
    }
    wr16(m, a1 + 0x480u, (uint16_t)(d1 + d1));       /* 0x002F78/0x002F7A */
out:
    r->d[0] = d0_save;
    r->d[1] = d1_save;
    r->d[2] = d2_save;
    r->a[0] = a0_save;
    r->a[1] = a1_save;
    r->a[2] = a2_save;
}

/* 0x0C4038: shared re-arm tail of the 0x0C3EF0/0x0C408A state: emit the step
 * sprite (0x0034A0 / 0x002F3A), unlink the neighbours (0x0C412E/0x0C4146) and
 * advance the frame script table at 0x0C41AC. Ends with its rts. */
static void Tail_C4038(const f3mem_t *m, f3regs_t *r, uint32_t a5, uint32_t a2,
                       uint32_t a1, uint32_t d0_in)
{
    uint32_t d0 = set_word(0u, (uint16_t)d0_in);     /* 0x0C4038 movem.l d0-d3/a0-a1 */
    uint32_t d1, d2, d3, a0;

    d3 = set_word(r->d[3], (uint16_t)d0_in);         /* 0x0C403C move.w d0,d3 */
    d1 = set_word(r->d[1], rd16(m, a2 + 0x000Cu));   /* 0x0C403E */
    push_long(m, r, 0x0C4048u);                      /* 0x0C4042 jsr 0x0BDE8C */
    d0 = Sub_BDE8C(m, a5, d0, d1, &a0);
    r->a[7] += 4u;
    d2 = set_word(r->d[2], rd16(m, a1 + 2u * (uint16_t)d0)); /* 0x0C4048 */
    d3 = set_word(d3, (uint16_t)(rd16(m, a2 + 0x0008u) -
                                 rd16(m, a5 + 0x1BC4u)));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - 0x00A0u)); /* 0x0C4050 */
    d3 = set_word(d3, (uint16_t)((int16_t)(uint16_t)d3 >> 1)); /* 0x0C4054 */
    push_word(m, r, (uint16_t)d3);                   /* 0x0C4056 */
    push_word(m, r, (uint16_t)d2);                   /* 0x0C4058 */
    push_long(m, r, 0x0C4060u);                      /* 0x0C405A jsr 0x0034A0 */
    Sub_34A0(m, r, a5, (uint16_t)d2, (uint16_t)d3);
    r->a[7] += 4u;                                   /* 0x0C4060 */
    push_word(m, r, (uint16_t)d2);                   /* 0x0C4062 */
    push_long(m, r, 0x0C406Au);                      /* 0x0C4064 jsr 0x002F3A */
    Sub_2F3A(m, r, a5, (uint16_t)d2);
    r->a[7] += 2u;                                   /* 0x0C406A */
    /* 0x0C406C movem.l (a7)+,d0-d3/a0-a1 */
    wr16(m, a2 + 0x0008u, (uint16_t)d0_in);          /* 0x0C4070 */
    d0 = d0_in;                                      /* restored D0 */
    wr16(m, a2 + 0x0002u, 0xFFFFu);                  /* 0x0C4074 */
    wr16(m, a2 + 0x0014u, 0u);                       /* 0x0C407A */
    wr8(m, a2 + 0x001Eu, 0u);                        /* 0x0C407E sf.b */
    push_long(m, r, 0x0C4082u);                      /* 0x0C4082 bsr 0x0C412E */
    Sub_C412E(m, r, r->a[4], a2);
    r->a[7] += 4u;
    push_long(m, r, 0x0C4086u);                      /* 0x0C4086 bsr 0x0C4146 */
    Sub_C4146(m, r, a5);
    r->a[7] += 4u;
    wr16(m, a2 + 0x0014u,                            /* 0x0C408A subq.w #1 */
         (uint16_t)(rd16(m, a2 + 0x0014u) - 1u));
    if ((int16_t)rd16(m, a2 + 0x0014u) > 0)          /* 0x0C408E bgt 0x0C40AA */
        goto step_b;
    wr16(m, a2 + 0x0002u,                            /* 0x0C4090 addq.w #1 */
         (uint16_t)(rd16(m, a2 + 0x0002u) + 1u));
    {
        uint16_t n = rd16(m, a2 + 0x0002u);          /* 0x0C4094 */
        uint32_t tbl = 0x0C41ACu;                    /* 0x0C4098 lea */

        if ((int32_t)rd32(m, tbl + 8u * n) < 0) {    /* 0x0C409C bmi 0x0C4126 */
            wr32(m, r->a[3], 0u);                    /* 0x0C3EDA clr.l (a3) */
            wr16(m, a2, 0u);                         /* 0x0C3EDA clr.w (a2) */
            r->a[7] += 4u;                           /* rts */
            return;
        }
        wr16(m, a2 + 0x0014u, rd16(m, tbl + 4u + 8u * n)); /* 0x0C40A4 */
    }
step_b:                                              /* 0x0C40AA */
    d0 = set_word(d0, rd16(m, a2 + 0x0002u));        /* 0x0C40AA move.w 2(a2),d0 */
    /* 0x0C40AE lea.l ([$c41ac, pc, d0.w * 8]), a1: pre-indexed memory
     * indirect -- fetch the script pointer stored in the table entry. */
    a1 = rd32(m, 0x0C41ACu + 8u * (uint32_t)(int32_t)(int16_t)(uint16_t)d0);
    d1 = set_word(0u, rd8(m, a2 + 0x0001u));         /* 0x0C40B6/0x0C40B8 */
    d2 = set_word(r->d[2], rd16(m, a2 + 0x0008u));   /* 0x0C40BC */
    d3 = set_word(r->d[3], rd16(m, a2 + 0x000Cu));   /* 0x0C40C0 */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BC4u)));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - rd16(m, a5 + 0x1BC6u)));
    if (rd8(m, a2 + 0x001Fu) != 0u) {                /* 0x0C40CC beq 0x0C40DE */
        a0 = rd32(m, a2 + 0x0018u);                  /* 0x0C40D2 */
        wr16(m, a2 + 0x000Cu,                        /* 0x0C40D6/0x0C40DA */
             (uint16_t)(rd16(m, a2 + 0x000Cu) + rd16(m, a0 + 0x001Au)));
    }
    if (rd8(m, a2 + 0x0001u) == 0u)                  /* 0x0C40DE */
        d2 = set_word(d2, (uint16_t)((uint16_t)d2 - 0x0010u)); /* 0x0C40E6 */
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - 0x001Bu)); /* 0x0C40EA */
    a0 = r->a[0];                                    /* whatever 0x0C412E left */
    if (rd16(m, a5 + 0x1CF4u) != 0u) {               /* 0x0C40EE beq 0x0C4118 */
        a0 = rd32(m, a5 + 0x1D08u);                  /* 0x0C40F4 */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C40F8 */
        a0 -= 2u; wr16(m, a0, (uint16_t)d1);         /* 0x0C40FC */
        a0 -= 2u; wr16(m, a0, 0x0044u);              /* 0x0C40FE */
        a0 -= 2u; wr16(m, a0, (uint16_t)d3);         /* 0x0C4102 */
        a0 -= 2u; wr16(m, a0, (uint16_t)d2);         /* 0x0C4104 */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C4106 */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C410A */
        a0 -= 4u; wr32(m, a0, a1);                   /* 0x0C410E move.l a1,-(a0) */
        wr16(m, a5 + 0x1CF4u,                        /* 0x0C4110 */
             (uint16_t)(rd16(m, a5 + 0x1CF4u) - 1u));
        wr32(m, a5 + 0x1D08u, a0);                   /* 0x0C4114 */
    }
    wr32(m, a2 + 0x0004u, 0x000C408Au);              /* 0x0C4118 */
    r->d[0] = d0;
    r->a[0] = a0;
    r->a[1] = a1;
    r->d[1] = d1;                                    /* 0x0C4120 rts */
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[7] += 4u;
}

/* 0x0C43D8: re-arm tail of the 0x0C4352 hand-off state; same shape as
 * 0x0C4038 but driven by the 0x0C4200 script table. Ends with its rts. */
static void Tail_C43D8(const f3mem_t *m, f3regs_t *r, uint32_t a5, uint32_t a2,
                       uint32_t a1, uint32_t d0_in)
{
    uint32_t d0 = d0_in, d1, d2, d3, a0;

    wr16(m, a2 + 0x0014u,                            /* 0x0C43D8 subq.w #1 */
         (uint16_t)(rd16(m, a2 + 0x0014u) - 1u));
    if ((int16_t)rd16(m, a2 + 0x0014u) > 0)          /* 0x0C43DC bgt 0x0C43F8 */
        goto step;
    wr16(m, a2 + 0x0002u,                            /* 0x0C43DE addq.w #1 */
         (uint16_t)(rd16(m, a2 + 0x0002u) + 1u));
    {
        uint16_t n = rd16(m, a2 + 0x0002u);          /* 0x0C43E2 */
        uint32_t tbl = 0x0C4200u;                    /* 0x0C43E6 lea $c4200(pc) */

        if ((int32_t)rd32(m, tbl + 8u * n) < 0) {    /* 0x0C43EA bmi 0x0C4126 */
            wr32(m, r->a[3], 0u);                    /* 0x0C3EDA clr.l (a3) */
            wr16(m, a2, 0u);                         /* 0x0C3EDA clr.w (a2) */
            r->a[7] += 4u;
            return;
        }
        wr16(m, a2 + 0x0014u, rd16(m, tbl + 4u + 8u * n)); /* 0x0C43F2 */
    }
step:                                                /* 0x0C43F8 */
    d0 = set_word(d0, rd16(m, a2 + 0x0002u));
    /* 0x0C43FC lea.l ([$c4200, pc, d0.w * 8]), a1: pre-indexed memory
     * indirect -- fetch the script pointer stored in the table entry. */
    a1 = rd32(m, 0x0C4200u + 8u * (uint32_t)(int32_t)(int16_t)(uint16_t)d0);
    d1 = set_word(0u, rd8(m, a2 + 0x0001u));         /* 0x0C4402/0x0C4404 */
    d2 = set_word(r->d[2], rd16(m, a2 + 0x0008u));   /* 0x0C4408 */
    d3 = set_word(r->d[3], rd16(m, a2 + 0x000Cu));   /* 0x0C440C */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BC4u)));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - rd16(m, a5 + 0x1BC6u)));
    if (rd8(m, a2 + 0x001Fu) != 0u) {                /* 0x0C4418 beq 0x0C442A */
        a0 = rd32(m, a2 + 0x0018u);                  /* 0x0C441E */
        wr16(m, a2 + 0x000Cu,                        /* 0x0C4422/0x0C4426 */
             (uint16_t)(rd16(m, a2 + 0x000Cu) + rd16(m, a0 + 0x001Au)));
    }
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - 5u)); /* 0x0C442A subq.w #5 */
    if (rd8(m, a2 + 0x0001u) != 0u)                  /* 0x0C442C */
        d2 = set_word(d2, (uint16_t)((uint16_t)d2 - 6u)); /* 0x0C4434 */
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 + 1u)); /* 0x0C4436 addq.w #1 */
    a0 = r->a[0];                                    /* whatever 0x0C4146 left */
    if (rd16(m, a5 + 0x1CF4u) != 0u) {               /* 0x0C4438 beq 0x0C4462 */
        a0 = rd32(m, a5 + 0x1D08u);                  /* 0x0C443E */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C4442 */
        a0 -= 2u; wr16(m, a0, (uint16_t)d1);         /* 0x0C4446 */
        a0 -= 2u; wr16(m, a0, 0x0044u);              /* 0x0C4448 */
        a0 -= 2u; wr16(m, a0, (uint16_t)d3);         /* 0x0C444C */
        a0 -= 2u; wr16(m, a0, (uint16_t)d2);         /* 0x0C444E */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C4450 */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C4454 */
        a0 -= 4u; wr32(m, a0, a1);                   /* 0x0C4458 move.l a1,-(a0) */
        wr16(m, a5 + 0x1CF4u,                        /* 0x0C445A */
             (uint16_t)(rd16(m, a5 + 0x1CF4u) - 1u));
        wr32(m, a5 + 0x1D08u, a0);                   /* 0x0C445E */
    }
    wr32(m, a2 + 0x0004u, 0x000C43D8u);              /* 0x0C4462 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[7] += 4u;                                   /* 0x0C446A rts */
}

int Ea_0C42A0(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a5 = r->a[5];
    const uint32_t a2 = r->a[2];
    const uint32_t a3 = r->a[3];
    const uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t d4 = r->d[4], d5 = r->d[5], d6 = r->d[6];
    uint32_t a0 = r->a[0], a1 = r->a[1];
    uint16_t d4w = (uint16_t)d4;
    int path;

    /* 0x0C42A0 bgt.w / 0x0C42A4 bne.w: both test the flags the 0x0BD546 call
     * before this entry left behind. D0.w = 1/2 is that callee's "record
     * found" moveq (Z=0, N=0: bgt), D0.w = 0 its "no record" moveq (Z=1:
     * fall through), and anything else the restored coordinate word of the
     * grid-bit path, whose ext.w left N=1 (Z=0, N!=V: bne). */
    if ((uint16_t)d0 != 0u) {
        if ((uint16_t)d0 == 1u || (uint16_t)d0 == 2u)
            goto path_431e;                          /* 0x0C42A0 bgt */
        goto path_4352;                              /* 0x0C42A4 bne */
    }

    /* ---- 0x0C42A8: four more integration passes ------------------------ */
    for (;;) {
        d4w = (uint16_t)(d4w - 1u);                  /* 0x0C42A8 dbra d4,0x0C428A */
        if (d4w == 0xFFFFu)
            break;
        /* 0x0C428A add.l d5,8(a2) ; 0x0C428E add.l d6,0xc(a2) */
        wr32(m, a2 + 0x0008u, rd32(m, a2 + 0x0008u) + d5);
        wr32(m, a2 + 0x000Cu, rd32(m, a2 + 0x000Cu) + d6);
        d1 = set_word(d1, rd16(m, a2 + 0x000Cu));    /* 0x0C4292 */
        d0 = set_word(d0, rd16(m, a2 + 0x0008u));    /* 0x0C4296 */
        r->d[0] = d0;                                /* 0x0C429A jsr 0x0BD546 */
        r->d[1] = d1;
        push_long(m, r, 0x0C42A0u);
        path = Sub_BD546(m, r, a5, &d0, &d1, &a0);
        if (path == 1)
            goto path_431e;                          /* 0x0C42A0 bgt */
        if (path == 2)
            goto path_4352;                          /* 0x0C42A4 bne */
    }

    /* ---- 0x0C42AC: floor-relative X, sprite command ------------------- */
    d1 = set_word(d1, (uint16_t)(rd16(m, a2 + 0x0008u) -
                                 rd16(m, a5 + 0x1BC4u)));
    if (rd8(m, a2 + 0x0001u) == 0u) {                /* 0x0C42B4 tst.b 1(a2) */
        if ((int16_t)(uint16_t)d1 > 0x14F)           /* 0x0C42BA cmp.w #$14f */
            goto path_4122;
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 - 0x000Fu)); /* 0x0C42C2 */
        d3 = 1u;                                     /* 0x0C42C6 moveq #1,d3 */
    } else {
        if ((int16_t)(uint16_t)d1 < -16)             /* 0x0C42CA cmp.w #$fff0 */
            goto path_4122;
        d3 = 0u;                                     /* 0x0C42D2 moveq #0,d3 */
    }
    d2 = set_word(d2, (uint16_t)((uint16_t)(rd16(m, a2 + 0x000Cu) -
                                            rd16(m, a5 + 0x1BC6u)) - 1u));
    d0 = 0x0003FBD2u;                                /* 0x0C42DE */
    if (rd16(m, a2 + 0x0016u) != 0u)                 /* 0x0C42E4 beq 0x0C42F0 */
        d0 = 0x0003FBEAu;                            /* 0x0C42EA */
    if (rd16(m, a5 + 0x1CF2u) != 0u) {               /* 0x0C42F0 beq 0x0C4314 */
        a0 = rd32(m, a5 + 0x1D04u);                  /* 0x0C42F6 */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C42FA clr.w -(a0) */
        a0 -= 2u; wr16(m, a0, (uint16_t)d3);         /* 0x0C42FC */
        a0 -= 2u; wr16(m, a0, 0x0044u);              /* 0x0C42FE */
        a0 -= 2u; wr16(m, a0, (uint16_t)d2);         /* 0x0C4302 */
        a0 -= 2u; wr16(m, a0, (uint16_t)d1);         /* 0x0C4304 */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C4306 clr.w -(a0) */
        a0 -= 2u; wr16(m, a0, 0u);                   /* 0x0C4308 clr.w -(a0) */
        a0 -= 4u; wr32(m, a0, d0);                   /* 0x0C430A move.l d0,-(a0) */
        wr16(m, a5 + 0x1CF2u,                        /* 0x0C430C subq.w #1 */
             (uint16_t)(rd16(m, a5 + 0x1CF2u) - 1u));
        wr32(m, a5 + 0x1D04u, a0);                   /* 0x0C4310 */
    }
    wr32(m, a2 + 0x0004u, 0x000C426Au);              /* 0x0C4314 */
    goto done;                                       /* 0x0C431C rts */

    /* ---- 0x0C431E: the first probe box hit --------------------------- */
path_431e:
    wr32(m, a2 + 0x0018u, a0);                       /* 0x0C431E */
    d0 = set_word(d0, (uint16_t)(rd16(m, a2 + 0x0008u) - 2u)); /* 0x0C4322 */
    if (rd8(m, a2 + 0x0001u) != 0u)                  /* 0x0C4328 */
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 + 4u)); /* 0x0C432E */
    d1 = set_word(d1, rd16(m, a2 + 0x000Cu));        /* 0x0C4330 */
    r->d[0] = d0;
    r->d[1] = d1;
    push_long(m, r, 0x0C433Au);                      /* 0x0C4334 jsr 0x0BD546 */
    if (Sub_BD546(m, r, a5, &d0, &d1, &a0) == 0)
        goto path_4012;                              /* 0x0C433A beq */
    wr8(m, a2 + 0x001Fu, 0xFFu);                     /* 0x0C433E st.b 1f(a2) */
    a1 = rd32(m, 0x000C4DD2u);                       /* 0x0C4342 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));        /* 0x0C4348 */
    d1 = set_word(d1, rd16(m, a2 + 0x000Cu));        /* 0x0C434C */
    goto tail_4394;                                  /* 0x0C4350 bra */

    /* ---- 0x0C4352: the second probe box hit -------------------------- */
path_4352:
    d0 = set_word(d0, (uint16_t)(rd16(m, a2 + 0x0008u) - 2u)); /* 0x0C4352 */
    if (rd8(m, a2 + 0x0001u) != 0u)                  /* 0x0C4358 */
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 + 4u)); /* 0x0C435E */
    d1 = set_word(d1, rd16(m, a2 + 0x000Cu));        /* 0x0C4360 */
    r->d[0] = d0;
    r->d[1] = d1;
    push_long(m, r, 0x0C436Au);                      /* 0x0C4364 jsr 0x0BD546 */
    if (Sub_BD546(m, r, a5, &d0, &d1, &a0) == 0)
        goto path_4022;                              /* 0x0C436A beq */
    a1 = rd32(m, 0x000C4DFCu);                       /* 0x0C436E */
    d0 = rd32(m, a2 + 0x0008u);                      /* 0x0C4374 */
    d2 = set_word(d2, rd16(m, a2 + 0x0010u));        /* 0x0C4378 */
    d2 = (uint32_t)(int32_t)(int16_t)(uint16_t)d2;   /* 0x0C437C ext.l */
    d2 <<= 2;                                        /* 0x0C437E lsl.l #2 */
    d0 -= d2;                                        /* 0x0C4380 */
    d0 = (d0 << 16) | (d0 >> 16);                    /* 0x0C4382 swap d0 */
    d1 = rd32(m, a2 + 0x000Cu);                      /* 0x0C4384 */
    d2 = set_word(d2, rd16(m, a2 + 0x0012u));        /* 0x0C4388 */
    d2 = (uint32_t)(int32_t)(int16_t)(uint16_t)d2;   /* 0x0C438C ext.l */
    d2 <<= 2;                                        /* 0x0C438E lsl.l #2 */
    d1 -= d2;                                        /* 0x0C4390 */
    d1 = (d1 << 16) | (d1 >> 16);                    /* 0x0C4392 swap d1 */

tail_4394:                                           /* 0x0C4394 */
    push_long(m, r, 0x0C439Au);                      /* 0x0C4394 jsr 0x0BDE8C */
    d0 = Sub_BDE8C(m, a5, d0, d1, &a0);
    r->a[7] += 4u;
    d2 = set_word(d2, rd16(m, a1 + 2u * (uint16_t)d0)); /* 0x0C439A */
    d3 = set_word(d3, (uint16_t)(rd16(m, a2 + 0x0008u) -
                                 rd16(m, a5 + 0x1BC4u)));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - 0x00A0u)); /* 0x0C43A6 */
    d3 = set_word(d3, (uint16_t)((int16_t)(uint16_t)d3 >> 1)); /* 0x0C43AA */
    push_word(m, r, (uint16_t)d3);                   /* 0x0C43AC */
    push_word(m, r, (uint16_t)d2);                   /* 0x0C43AE */
    push_long(m, r, 0x0C43B6u);                      /* 0x0C43B0 jsr 0x0034A0 */
    Sub_34A0(m, r, a5, (uint16_t)d2, (uint16_t)d3);
    r->a[7] += 4u;                                   /* 0x0C43B6 */
    push_word(m, r, (uint16_t)d2);                   /* 0x0C43B8 */
    push_long(m, r, 0x0C43C0u);                      /* 0x0C43BA jsr 0x002F3A */
    Sub_2F3A(m, r, a5, (uint16_t)d2);
    r->a[7] += 2u;                                   /* 0x0C43C0 */
    wr16(m, a2 + 0x0002u, 0xFFFFu);                  /* 0x0C43C2 */
    wr16(m, a2 + 0x0014u, 0u);                       /* 0x0C43C8 */
    wr8(m, a2 + 0x001Eu, 0u);                        /* 0x0C43CC sf.b */
    push_long(m, r, 0x0C43D0u);                      /* 0x0C43D0 bsr 0x0C412E */
    Sub_C412E(m, r, r->a[4], a2);
    r->a[7] += 4u;
    push_long(m, r, 0x0C43D4u);                      /* 0x0C43D4 bsr 0x0C4146 */
    Sub_C4146(m, r, a5);
    r->a[7] += 4u;
    Tail_C43D8(m, r, a5, a2, a1, d0);                /* 0x0C43D8 */
    return 0;

    /* ---- 0x0C4012 / 0x0C4022: the retry probe found nothing ---------- */
path_4012:
    wr8(m, a2 + 0x001Fu, 0xFFu);                     /* 0x0C4012 st.b 1f(a2) */
    a1 = rd32(m, 0x000C4DD2u);                       /* 0x0C4016 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));        /* 0x0C401C */
    goto tail_4038;
path_4022:
    a1 = rd32(m, 0x000C4DFCu);                       /* 0x0C4022 */
    d0 = set_word(d0, rd16(m, a2 + 0x0008u));        /* 0x0C4028 */
tail_4038:                                           /* 0x0C4038 */
    Tail_C4038(m, r, a5, a2, a1, d0);
    return 0;

    /* ---- 0x0C4122: out of range: unlink from A4 and drop the object --- */
path_4122:
    push_long(m, r, 0x0C4126u);                      /* 0x0C4122 bsr 0x0C412E */
    Sub_C412E(m, r, r->a[4], a2);
    r->a[7] += 4u;
    /* 0x0C4126 bra 0x0C3EDA: clr.l (a3) ; clr.w (a2) ; rts */
    wr32(m, a3, 0u);
    wr16(m, a2, 0u);

done:
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = set_word(d4, d4w);
    r->d[5] = d5;
    r->d[6] = d6;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[7] = sp + 4u;                               /* the rts pops the PC */
    return 0;
}
