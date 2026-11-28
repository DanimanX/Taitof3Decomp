/* ROM 0x0C44CA: per-frame step of the walker object at A2 -- integrates its
 * $10/$12 velocity, resolves the floor it stands on with the tile/object
 * probes 0x0BD546/0x0BD8A4/0x0BD8CE/0x0BD9C8/0x0BD9F2, advances the $14 walk
 * frame from the 8-byte table at 0x0C466A, queues one sprite and re-arms
 * itself with 0x0C44CA at $4(A2). */
#include "ea_auto.h"

/* CMP2.W: compares Dn with the word pair at ea (lower) / ea+2 (upper) as a
 * signed range; the MAME/Musashi chk2cmp2 sets C when Dn is OUTSIDE the
 * bounds, so BCS = outside and BCC = inside (bounds inclusive). */
static int in_range(uint16_t v, uint16_t lo, uint16_t hi)
{
    return (int16_t)v >= (int16_t)lo && (int16_t)v <= (int16_t)hi;
}

/* ---- 0x0BD516: solid-tile probe under (d0.w,d1.w). Z=0 means "solid". ---- */
static uint32_t h_bd516(const f3mem_t *m, uint32_t a5, uint32_t d0, uint32_t d1,
                        uint32_t *a0p, int *z)
{
    uint16_t w;

    /* 0x0BD516 sub.w $998(a5),d0 ; 0x0BD51A asr.w #2,d0 */
    w = (uint16_t)(((int16_t)(uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x998u))) >> 2);
    d0 = set_word(d0, w);
    if ((uint16_t)w >= rd16(m, a5 + 0x99Cu)) {          /* 0x0BD51C cmp / bcc */
        *z = 1;
        return 0;                                       /* 0x0BD542 moveq #0,d0 */
    }
    /* 0x0BD522 sub.w $99a(a5),d1 ; 0x0BD526 asr.w #1,d1 */
    w = (uint16_t)(((int16_t)(uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x99Au))) >> 1);
    if ((uint16_t)w >= rd16(m, a5 + 0x99Eu)) {          /* 0x0BD528 cmp / bcc */
        *z = 1;
        return 0;
    }
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;      /* 0x0BD52E ext.l d0 */
    {
        uint32_t p = rd32(m, a5 + 0x994u);              /* 0x0BD530 movea.l $994(a5),a0 */
        /* 0x0BD534 adda.w (a0,d1.w*2),a0: the row table holds signed offsets
         * from its own base, added as a word. */
        p += (uint32_t)(int32_t)(int16_t)
             rd16(m, p + (uint32_t)((int32_t)(int16_t)w * 2));
        *a0p = p;
        /* 0x0BD538 bftst (a0){D0:1}: the extension word 0x0801 is Do=1 with
         * offset register D0 (the x cell index) and width 1 (the low 5 bits
         * hold the width itself). The bit counts from the MSB of the
         * addressed byte, so offset >= 8 walks on in the tile bitmap. */
        uint32_t off = (uint16_t)d0;
        *z = (rd8(m, p + (off >> 3)) & (0x80u >> (off & 7u))) == 0u;
    }
    d0 = set_word(d0, *z ? 0x0000u : 0xFFFFu);          /* 0x0BD53C sne.b / ext.w d0 */
    return d0;
}

/* ---- 0x0BD546: 0 = the (d0.w,d1.w) position is free, 1/2 = blocked. ---- */
static int h_bd546(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t d0_in = r->d[0], d1_in = r->d[1];
    uint16_t d0w, d1w, cnt;
    uint32_t a0 = r->a[0];
    int z;

    /* 0x0BD546 movem.w d0-d1,-(a7) ; 0x0BD54A bsr 0x0BD516 */
    r->d[0] = h_bd516(m, a5, r->d[0], r->d[1], &a0, &z);
    /* 0x0BD54C movem.w (a7)+,d0-d1: a 16-bit MOVEM *load* sign-extends each
     * word over the whole data register on the 68020, so D0 and D1 lose their
     * previous high halves here (this is observable in the returned D1). */
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)d0_in;
    r->d[1] = (uint32_t)(int32_t)(int16_t)(uint16_t)d1_in;
    r->a[0] = a0;
    if (!z)
        return 1;   /* solid tile: 0x0BD550 bne -> 0x0BD59C rts, D0 = the restored word */

    if ((int16_t)rd16(m, 0x0041B818u) < 0) {            /* 0x0BD552 tst.w / bmi */
        r->d[0] = 0;                                    /* 0x0BD59A moveq #0,d0 */
        return 0;
    }

    d0w = (uint16_t)r->d[0];
    d1w = (uint16_t)r->d[1];
    cnt = rd16(m, 0x0041B818u);                         /* 0x0BD55E move.w, d2 */
    {
        uint32_t a1 = 0x0041B81Cu;                      /* 0x0BD564 lea.l $41b81c,a1 */
        for (;;) {
            uint32_t a0o;
            a0 = rd32(m, a1);                           /* 0x0BD56A movea.l (a1)+,a0 */
            a1 += 4u;
            a0o = a0;
            /* 0x0BD56C bcs -> box skipped; 0x0BD574 bcc -> point is inside */
            if (in_range(d0w, rd16(m, a0o + 0x34u), rd16(m, a0o + 0x36u))
                && in_range(d1w, rd16(m, a0o + 0x3Cu), rd16(m, a0o + 0x3Eu))) {
                r->a[0] = a0o;
                r->d[0] = 1;                            /* 0x0BD5A6 moveq #1,d0 */
                return 1;
            }
            /* 0x0BD57C: box B, only while the object holds a live box */
            if (rd32(m, a0o + 0x24u) != 0
                && in_range(d0w, rd16(m, a0o + 0x24u), rd16(m, a0o + 0x26u))
                && in_range(d1w, rd16(m, a0o + 0x2Cu), rd16(m, a0o + 0x2Eu))) {
                r->a[0] = a0o;
                r->d[0] = 2;                            /* 0x0BD59E moveq #2,d0 */
                return 1;
            }
            cnt = (uint16_t)(cnt - 1u);                 /* 0x0BD592 dbra d2 */
            if (cnt == 0xFFFFu) {
                r->a[0] = a0o;
                r->d[0] = 0;                            /* 0x0BD596/0x0BD59A */
                return 0;
            }
        }
    }
}

/* ---- 0x0BD90C / 0x0BDA30: walk the object list at 0x41B81C.
 * hi = 1 selects the 0x0BD90C loop (bge/blt on the "upper" edge), hi = 0 the
 * 0x0BDA30 loop (ble/bgt on the "lower" edge). d3 = 0 takes the first loop,
 * any other value the second one (0x0BD96A / 0x0BDA8E). */
static uint32_t h_scan_list(const f3mem_t *m, uint32_t d0, uint16_t d1w, uint16_t d2,
                            uint16_t d3, int hi, uint32_t *a0p)
{
    /* {cmp2 lower bound, d0/d1 compare offset, result offset} per block;
     * a0 is set to obj + cmp2 lower bound. */
    static const uint16_t off[2][2][2][3] = {
        {   /* hi = 0: the 0x0BDA30 family (d3 = 0, then the 0x0BDA8E one) */
            { { 0x2C, 0x24, 0x24 }, { 0x3C, 0x34, 0x34 } },
            { { 0x24, 0x2C, 0x2C }, { 0x34, 0x3C, 0x3C } },
        },
        {   /* hi = 1: the 0x0BD90C family (d3 = 0, then the 0x0BD96A one) */
            { { 0x2C, 0x26, 0x26 }, { 0x3C, 0x36, 0x36 } },
            { { 0x24, 0x2E, 0x2E }, { 0x34, 0x3E, 0x3E } },
        },
    };
    const uint16_t (*o)[3] = off[hi][d3 != 0];
    uint32_t a0 = *a0p;
    uint32_t a2 = 0x0041B81Cu;                          /* 0x0BD90C lea.l $41b81c,a2 */
    uint16_t cnt = rd16(m, 0x0041B818u);                /* 0x0BD916 / 0x0BD96A */

    if ((int16_t)cnt >= 0) {                            /* bmi -> done */
        for (;;) {
            uint32_t obj = rd32(m, a2);                 /* movea.l (a2)+,a1 */
            a2 += 4u;
            /* block 0: guarded by tst.l $24(a1) */
            if (rd32(m, obj + 0x24u) != 0) {
                int16_t v = (int16_t)rd16(m, obj + o[0][1]);
                int hit = hi ? ((int16_t)(uint16_t)d0 < v && (int16_t)d1w >= v)
                             : ((int16_t)(uint16_t)d0 > v && (int16_t)d1w <= v);
                if (in_range(d2, rd16(m, obj + o[0][0]), rd16(m, obj + o[0][0] + 2u)) && hit) {
                    d0 = set_word(d0, rd16(m, obj + o[0][2]));
                    a0 = obj + o[0][0];
                }
            }
            /* block 1 */
            {
                int16_t v = (int16_t)rd16(m, obj + o[1][1]);
                int hit = hi ? ((int16_t)(uint16_t)d0 < v && (int16_t)d1w >= v)
                             : ((int16_t)(uint16_t)d0 > v && (int16_t)d1w <= v);
                if (in_range(d2, rd16(m, obj + o[1][0]), rd16(m, obj + o[1][0] + 2u)) && hit) {
                    d0 = set_word(d0, rd16(m, obj + o[1][2]));
                    a0 = obj + o[1][0];
                }
            }
            cnt = (uint16_t)(cnt - 1u);                 /* dbra d3 */
            if (cnt == 0xFFFFu)
                break;
        }
    }
    *a0p = a0;
    return d0;
}

/* ---- 0x0BD8A4/0x0BD8CE/0x0BD9C8/0x0BD9F2: grid lookup plus list scan.
 * flip = 0 probes y first (0x998/0x99C), flip = 1 probes x first
 * (0x99A/0x99E); base is the row-pointer table, err the "no grid" value. */
static uint32_t h_probe(const f3mem_t *m, uint32_t a5, uint32_t d0, uint32_t d1,
                        int hi, int flip, uint32_t base, uint16_t err, uint16_t d3,
                        uint32_t *a0p)
{
    uint16_t d2 = (uint16_t)d0;                         /* 0x0BD8AA move.w d0,d2 */
    uint16_t d4;
    uint32_t a0;

    if (!flip) {
        d4 = (uint16_t)(((int16_t)(uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x998u))) >> 2);
        if ((uint16_t)d4 >= rd16(m, a5 + 0x99Cu))
            goto err;
        d4 = (uint16_t)(((int16_t)(uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x99Au))) >> 1);
        if ((uint16_t)d4 >= rd16(m, a5 + 0x99Eu))
            goto err;
    } else {
        d4 = (uint16_t)(((int16_t)(uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x99Au))) >> 1);
        if ((uint16_t)d4 >= rd16(m, a5 + 0x99Eu))
            goto err;
        d4 = (uint16_t)(((int16_t)(uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x998u))) >> 2);
        if ((uint16_t)d4 >= rd16(m, a5 + 0x99Cu))
            goto err;
    }
    a0 = rd32(m, a5 + base);                            /* movea.l $98c/$984/$990/$988(a5),a0 */
    /* 0x0BD8F6 / 0x0BDA1A: walk the row list to the cell */
    d4 = rd16(m, a0 + (uint32_t)((int32_t)(int16_t)d4 * 2));
    d4 = (uint16_t)(-(int16_t)d4);                      /* 0x0BD8FA neg.w d4 */
    a0 += (uint32_t)(int32_t)(int16_t)
          rd16(m, a0 + (uint32_t)((int32_t)(int16_t)d4 * 2) - 2u);  /* 0x0BD8FC adda.w */
    /* 0x0BD900 subq.w #4,a0, then the 0x0BD902 loop body starts with addq.w
     * #4,a0: the walk advances a0 by 4 while the compare says "keep going". */
    a0 -= 4u;
    if (hi) {
        do {
            a0 += 4u;                                  /* 0x0BD902 addq.w #4,a0 */
        } while ((int16_t)d1 < (int16_t)rd16(m, a0));  /* 0x0BD904 / 0x0BD906 blt */
    } else {
        do {
            a0 += 4u;                                  /* 0x0BDA26 addq.w #4,a0 */
        } while ((int16_t)d1 > (int16_t)rd16(m, a0));  /* 0x0BDA28 / 0x0BDA2A bgt */
    }
    d0 = set_word(d0, rd16(m, a0));                     /* 0x0BD908 move.w (a0)+,d0 */
    a0 += 2u;
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0);      /* 0x0BD90A adda.w (a0),a0 */
    goto scan;

err:
    d0 = set_word(d0, err);                             /* 0x0BD896 / 0x0BD9BC */
    a0 = 0x000BD5AEu;
scan:
    *a0p = a0;
    return h_scan_list(m, d0, (uint16_t)d1, d2, d3, hi, a0p);
}

/* 0x0BD8A4: y first, table $98C, no-grid value $8000, "low" scan loop. */
static uint32_t h_bd8a4(const f3mem_t *m, uint32_t a5, uint32_t d0, uint32_t d1,
                        uint32_t *a0p)
{
    return h_probe(m, a5, d0, d1, 1, 0, 0x98Cu, 0x8000u, 0x0000u, a0p);
}

/* 0x0BD8CE: x first, table $984, no-grid value $8000, "low" scan loop. */
static uint32_t h_bd8ce(const f3mem_t *m, uint32_t a5, uint32_t d0, uint32_t d1,
                        uint32_t *a0p)
{
    return h_probe(m, a5, d0, d1, 1, 1, 0x984u, 0x8000u, 0xFFFFu, a0p);
}

/* 0x0BD9C8: y first, table $990, no-grid value $7FFF, "high" scan loop. */
static uint32_t h_bd9c8(const f3mem_t *m, uint32_t a5, uint32_t d0, uint32_t d1,
                        uint32_t *a0p)
{
    return h_probe(m, a5, d0, d1, 0, 0, 0x990u, 0x7FFFu, 0x0000u, a0p);
}

/* 0x0BD9F2: x first, table $988, no-grid value $7FFF, "high" scan loop. */
static uint32_t h_bd9f2(const f3mem_t *m, uint32_t a5, uint32_t d0, uint32_t d1,
                        uint32_t *a0p)
{
    return h_probe(m, a5, d0, d1, 0, 1, 0x988u, 0x7FFFu, 0xFFFFu, a0p);
}

/* 0x0C3EDA: the shared tail both 0x0C4666 exits reach -- clear the object's
 * script long at (A3) and its +0 word, then return. */
static int kill_obj(const f3mem_t *m, f3regs_t *r)
{
    wr32(m, r->a[3], 0u);                               /* 0x0C3EDA clr.l (a3) */
    wr16(m, r->a[2], 0u);                               /* 0x0C3EDC clr.w (a2) */
    r->a[7] += 4u;                                      /* 0x0C3EDE rts */
    return 0;
}

int Ea_0C44CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t a2 = r->a[2];
    uint16_t w;

    /* 0x0C44CA subq.w #1,$2(a2) ; 0x0C44CE bmi.w 0x0C4666 (bra.w 0x0C3EDA) */
    w = (uint16_t)(rd16(m, a2 + 2u) - 1u);
    wr16(m, a2 + 2u, w);
    if (w & 0x8000u)
        return kill_obj(m, r);

    /* 0x0C44D2 .. 0x0C44E4: is the current cell free? */
    r->d[2] = set_word(r->d[2], rd16(m, a2 + 8u));
    r->d[3] = set_word(r->d[3], rd16(m, a2 + 0xCu));
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[3]);
    r->d[0] = set_word(r->d[0], (uint16_t)r->d[2]);
    if (h_bd546(m, r))
        return kill_obj(m, r);                          /* 0x0C44E4 bne.w */

    /* 0x0C44E8: $8(a2) += ($10(a2) << 8) */
    r->d[0] = set_word(r->d[0], rd16(m, a2 + 0x10u));
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
    r->d[0] <<= 8;
    wr32(m, a2 + 8u, rd32(m, a2 + 8u) + r->d[0]);
    /* 0x0C44F4 addi.w #$20,$12(a2) */
    wr16(m, a2 + 0x12u, (uint16_t)(rd16(m, a2 + 0x12u) + 0x20u));
    /* 0x0C44FA: $c(a2) += ($12(a2) << 8) */
    r->d[0] = set_word(r->d[0], rd16(m, a2 + 0x12u));
    r->d[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)r->d[0];
    r->d[0] <<= 8;
    wr32(m, a2 + 0xCu, rd32(m, a2 + 0xCu) + r->d[0]);
    /* 0x0C4506 move.w $8(a2),d4 ; 0x0C450A move.w $c(a2),d5 */
    r->d[4] = set_word(r->d[4], rd16(m, a2 + 8u));
    r->d[5] = set_word(r->d[5], rd16(m, a2 + 0xCu));

    /* 0x0C450E .. 0x0C4518 */
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[5]);
    r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
    if (!h_bd546(m, r))
        goto L45D2;                                     /* 0x0C4518 beq.w */
    {
        /* 0x0C451C: same spot with the old x */
        r->d[1] = set_word(r->d[1], (uint16_t)r->d[3]);
        r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
        if (!h_bd546(m, r))
            goto L4572;                                 /* 0x0C4526 beq.b */
        if (rd8(m, a2 + 1u) != 0) {                     /* 0x0C4528 tst.b $1(a2) */
            /* 0x0C453E */
            r->d[0] = set_word(r->d[0], (uint16_t)r->d[3]);
            r->d[1] = set_word(r->d[1], (uint16_t)r->d[2]);
            r->d[0] = h_bd9c8(m, a5, r->d[0], r->d[1], &r->a[0]);
            w = (uint16_t)((uint16_t)r->d[0] - 1u);     /* 0x0C4548 subq.w #1,d0 */
        } else {
            /* 0x0C452E */
            r->d[0] = set_word(r->d[0], (uint16_t)r->d[3]);
            r->d[1] = set_word(r->d[1], (uint16_t)r->d[2]);
            r->d[0] = h_bd8a4(m, a5, r->d[0], r->d[1], &r->a[0]);
            w = (uint16_t)((uint16_t)r->d[0] + 1u);     /* 0x0C4538 addq.w #1,d0 */
        }
        w = (uint16_t)(w + w);                          /* 0x0C453A/0x0C454A add.w d0,d0 */
        r->d[0] = set_word(r->d[0], w);
        /* 0x0C454C swap d0 ; 0x0C454E clr.w d0 */
        r->d[0] = (r->d[0] << 16) | (r->d[0] >> 16);
        r->d[0] &= 0xFFFF0000u;
        /* 0x0C4550 neg.l $8(a2) ; 0x0C4554 add.l d0,$8(a2) */
        wr32(m, a2 + 8u, (uint32_t)(0u - rd32(m, a2 + 8u)) + r->d[0]);
        /* 0x0C4558 neg.w $10(a2) ; 0x0C455C eori.b #1,$1(a2) */
        wr16(m, a2 + 0x10u, (uint16_t)(0u - rd16(m, a2 + 0x10u)));
        wr8(m, a2 + 1u, (uint8_t)(rd8(m, a2 + 1u) ^ 1u));
        /* 0x0C4562: the old x at the new y */
        r->d[1] = set_word(r->d[1], (uint16_t)r->d[5]);
        r->d[0] = set_word(r->d[0], (uint16_t)r->d[2]);
        if (!h_bd546(m, r))
            goto L45D2;                                 /* 0x0C456C beq.b */
        r->d[4] = set_word(r->d[4], rd16(m, a2 + 8u));  /* 0x0C456E move.w $8(a2),d4 */
    }

L4572:
    /* 0x0C4572 tst.w $12(a2) ; bpl 0x0C4588 */
    if ((int16_t)rd16(m, a2 + 0x12u) < 0) {
        /* 0x0C4578 */
        r->d[1] = set_word(r->d[1], (uint16_t)r->d[3]);
        r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
        r->d[0] = h_bd8ce(m, a5, r->d[0], r->d[1], &r->a[0]);
        w = (uint16_t)((uint16_t)r->d[0] + 1u);         /* 0x0C4582 addq.w #1,d0 */
    } else {
        /* 0x0C4588 */
        r->d[1] = set_word(r->d[1], (uint16_t)r->d[3]);
        r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
        r->d[0] = h_bd9f2(m, a5, r->d[0], r->d[1], &r->a[0]);
        w = (uint16_t)((uint16_t)r->d[0] - 1u);         /* 0x0C4592 subq.w #1,d0 */
    }
    w = (uint16_t)(w + w);                              /* 0x0C4584/0x0C4594 */
    r->d[0] = set_word(r->d[0], w);
    /* 0x0C4596 swap d0 ; clr.w d0 */
    r->d[0] = (r->d[0] << 16) | (r->d[0] >> 16);
    r->d[0] &= 0xFFFF0000u;
    /* 0x0C459A neg.l $c(a2) ; add.l d0,$c(a2) */
    wr32(m, a2 + 0xCu, (uint32_t)(0u - rd32(m, a2 + 0xCu)) + r->d[0]);
    /* 0x0C45A2 move.w $12(a2),d0 ; asr.w #1,d0 ; sub.w d0,$12(a2) */
    w = (uint16_t)(((int16_t)rd16(m, a2 + 0x12u)) >> 1);
    r->d[0] = set_word(r->d[0], w);
    wr16(m, a2 + 0x12u, (uint16_t)(rd16(m, a2 + 0x12u) - w));
    /* 0x0C45AC neg.w $12(a2) */
    wr16(m, a2 + 0x12u, (uint16_t)(0u - rd16(m, a2 + 0x12u)));
    /* 0x0C45B0 move.w $10(a2),d0 ; asr.w #2,d0 ; sub.w d0,$10(a2) */
    w = (uint16_t)(((int16_t)rd16(m, a2 + 0x10u)) >> 2);
    r->d[0] = set_word(r->d[0], w);
    wr16(m, a2 + 0x10u, (uint16_t)(rd16(m, a2 + 0x10u) - w));
    /* 0x0C45BA jsr $1338 */
    push_long(m, r, 0x0C45C0u);
    Ea_001338(m, r);
    /* 0x0C45C0 bftst -$785e(a5){2:2} ; bne 0x0C45D2 -- ext word 0x0082 is
     * offset 2, width 2, so bits 29-28 of the long at A5-0x785E. */
    if ((rd32(m, a5 - 0x785Eu) & 0x30000000u) == 0) {
        /* 0x0C45C8 neg.w $10(a2) ; 0x0C45CC eori.b #1,$1(a2) */
        wr16(m, a2 + 0x10u, (uint16_t)(0u - rd16(m, a2 + 0x10u)));
        wr8(m, a2 + 1u, (uint8_t)(rd8(m, a2 + 1u) ^ 1u));
    }

L45D2:
    /* 0x0C45D2: sprite position = object pos - camera - 7 */
    r->d[1] = set_word(r->d[1], rd16(m, a2 + 8u));
    r->d[2] = set_word(r->d[2], rd16(m, a2 + 0xCu));
    w = (uint16_t)((uint16_t)r->d[1] - rd16(m, a5 + 0x1BC4u));
    r->d[1] = set_word(r->d[1], w);
    w = (uint16_t)((uint16_t)r->d[2] - rd16(m, a5 + 0x1BC6u));
    r->d[2] = set_word(r->d[2], w);
    w = (uint16_t)((uint16_t)r->d[1] - 7u);             /* 0x0C45E2 subq.w #7,d1 */
    r->d[1] = set_word(r->d[1], w);
    w = (uint16_t)((uint16_t)r->d[2] - 7u);             /* 0x0C45E4 subq.w #7,d2 */
    r->d[2] = set_word(r->d[2], w);

    r->a[1] = 0x000C466Au;                              /* 0x0C45E6 lea.l $c466a(pc),a1 */

    /* 0x0C45EA subq.b #1,$15(a2) ; bpl 0x0C4620 */
    {
        uint8_t b = (uint8_t)(rd8(m, a2 + 0x15u) - 1u);
        wr8(m, a2 + 0x15u, b);
        if (b & 0x80u) {
            if (rd8(m, a2 + 1u) == 0) {                 /* 0x0C45F0 tst.b $1(a2) */
                uint8_t f = (uint8_t)(rd8(m, a2 + 0x14u) + 1u);  /* 0x0C45F6 */
                wr8(m, a2 + 0x14u, f);
                if (f >= 6u)
                    wr8(m, a2 + 0x14u, 0);              /* 0x0C4602 sf.b */
            } else {
                uint8_t f = (uint8_t)(rd8(m, a2 + 0x14u) - 1u);  /* 0x0C4608 */
                wr8(m, a2 + 0x14u, f);
                if (f & 0x80u)
                    wr8(m, a2 + 0x14u, 5);              /* 0x0C460E move.b #5 */
            }
            /* 0x0C4614 */
            r->d[0] = 0;                                /* moveq #0,d0 */
            r->d[0] = set_word(r->d[0], rd8(m, a2 + 0x14u));
            /* 0x0C461A move.b $5(a1,d0.w*8),$15(a2) */
            wr8(m, a2 + 0x15u,
                rd8(m, r->a[1] + (uint32_t)(uint16_t)r->d[0] * 8u + 5u));
        }
    }

    /* 0x0C4620 moveq #0,d0 ; move.b $14(a2),d0 */
    r->d[0] = 0;
    r->d[0] = set_word(r->d[0], rd8(m, a2 + 0x14u));
    /* 0x0C4626 move.w $6(a1,d0.w*8),d3 */
    r->d[3] = set_word(r->d[3],
                       rd16(m, r->a[1] + (uint32_t)(uint16_t)r->d[0] * 8u + 6u));
    /* 0x0C462A movea.l (a1,d0.w*8),a1 */
    r->a[1] = rd32(m, r->a[1] + (uint32_t)(uint16_t)r->d[0] * 8u);
    /* 0x0C462E move.b $1(a2),d0 ; 0x0C4632 eor.w d0,d3 */
    r->d[0] = set_word(r->d[0], rd8(m, a2 + 1u));
    r->d[3] = set_word(r->d[3], (uint16_t)((uint16_t)r->d[3] ^ (uint16_t)r->d[0]));

    /* 0x0C4634 tst.w $1cf4(a5) ; beq 0x0C465C */
    if (rd16(m, a5 + 0x1CF4u) != 0) {
        uint32_t a0 = rd32(m, a5 + 0x1D08u);            /* 0x0C463A movea.l $1d08(a5),a0 */
        a0 -= 2u; wr16(m, a0, (uint16_t)r->d[3]);       /* 0x0C463E */
        a0 -= 2u; wr16(m, a0, (uint16_t)r->d[3]);       /* 0x0C4640 */
        a0 -= 2u; wr16(m, a0, 0x0046u);                 /* 0x0C4642 move.w #$46 */
        a0 -= 2u; wr16(m, a0, (uint16_t)r->d[2]);       /* 0x0C4646 */
        a0 -= 2u; wr16(m, a0, (uint16_t)r->d[1]);       /* 0x0C4648 */
        a0 -= 2u; wr16(m, a0, 0x0000u);                 /* 0x0C464A */
        a0 -= 2u; wr16(m, a0, 0x0000u);                 /* 0x0C464E */
        a0 -= 4u; wr32(m, a0, r->a[1]);                 /* 0x0C4652 move.l a1,-(a0) */
        wr16(m, a5 + 0x1CF4u, (uint16_t)(rd16(m, a5 + 0x1CF4u) - 1u));  /* 0x0C4654 */
        wr32(m, a5 + 0x1D08u, a0);                      /* 0x0C4658 */
        r->a[0] = a0;
    }

    /* 0x0C465C move.l #$c44ca,$4(a2) ; 0x0C4664 rts */
    wr32(m, a2 + 4u, 0x000C44CAu);
    r->a[7] += 4u;
    return 0;
}
