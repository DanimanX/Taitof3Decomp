/* ROM 0x0C18F2: player/object floor-contact handler. When A4+0x42 holds no
 * hit it probes the floor grid at the three A4+0x8E/+0x90 box offsets and, on
 * a hit, publishes the record and its box words; a box code of 1 then runs the
 * A4+0x32 state checks that arm the 0x0CEDF6 call. */
#include "ea_auto.h"

/* cmp2.w (ea),Dn: 1 when the word is outside the signed {lo, hi} pair. */
static int OutsideBounds(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, addr), hi = (int16_t)rd16(m, addr + 2u);

    return (int16_t)v < lo || (int16_t)v > hi;
}

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

int Ea_0C18F2(const f3mem_t *m, f3regs_t *r)
{
    const uint32_t a4 = r->a[4];
    const uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d5 = r->d[5];
    uint32_t a0 = r->a[0];

    if ((int32_t)rd32(m, a4 + 0x009Cu) <= 0)         /* 0x0C18F2 ble 0x0C19E6 */
        goto ret;
    if ((int32_t)rd32(m, a4 + 0x0042u) > 0)          /* 0x0C18FA bgt 0x0C19E8 */
        goto have_record;
    if (rd16(m, rd32(m, a4 + 0x0072u) + 0x000Cu) != 0u) { /* 0x0C1902 beq 0x0C192C */
        d1 = set_word(d1, rd16(m, a4 + 0x00A4u));    /* 0x0C190E */
        d0 = set_word(d0, rd16(m, a4 + 0x00A0u));    /* 0x0C1912 */
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0x008Eu))); /* 0x0C1916 */
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0x0090u))); /* 0x0C191A */
        r->d[0] = d0;
        r->d[1] = d1;
        push_long(m, r, 0x0C1924u);                  /* 0x0C191E jsr 0x0BD546 */
        if (Sub_BD546(m, r, r->a[5], &d0, &d1, &a0) == 1) /* 0x0C1924 bgt */
            goto hit;
        goto ret;                                    /* 0x0C1928 bra 0x0C19E6 */
    }
    /* 0x0C192C: three clamped probes */
    d0 = set_word(d0, rd16(m, a4 + 0x00DAu));        /* 0x0C192C */
    d1 = set_word(d1, rd16(m, a4 + 0x00DCu));        /* 0x0C1930 */
    d5 = set_word(d5, rd16(m, a4 + 0x00E6u));        /* 0x0C1934 */
    if ((int16_t)(uint16_t)d5 > (int16_t)rd16(m, a4 + 0x00EEu)) /* 0x0C1938 ble */
        d5 = set_word(d5, rd16(m, a4 + 0x00EEu));    /* 0x0C193E */
    if ((int16_t)(uint16_t)d0 < (int16_t)(uint16_t)d5) /* 0x0C1942 bge */
        d0 = set_word(d0, (uint16_t)d5);             /* 0x0C1946 */
    d5 = set_word(d5, rd16(m, a4 + 0x00E8u));        /* 0x0C1948 */
    if ((int16_t)(uint16_t)d5 < (int16_t)rd16(m, a4 + 0x00F0u)) /* 0x0C194C bge */
        d5 = set_word(d5, rd16(m, a4 + 0x00F0u));    /* 0x0C1952 */
    if ((int16_t)(uint16_t)d0 > (int16_t)(uint16_t)d5) /* 0x0C1956 ble */
        d0 = set_word(d0, (uint16_t)d5);             /* 0x0C195A */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0x00A0u))); /* 0x0C195C */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0x00A4u))); /* 0x0C1960 */
    r->d[0] = d0;
    r->d[1] = d1;
    push_long(m, r, 0x0C196Au);                      /* 0x0C1964 jsr 0x0BD546 */
    if (Sub_BD546(m, r, r->a[5], &d0, &d1, &a0) == 1) /* 0x0C196A bgt 0x0C19AC */
        goto hit;
    d0 = set_word(d0, rd16(m, a4 + 0x00DEu));        /* 0x0C196C */
    d1 = set_word(d1, rd16(m, a4 + 0x00E0u));        /* 0x0C1970 */
    d5 = set_word(d5, rd16(m, a4 + 0x00E6u));        /* 0x0C1974 */
    if ((int16_t)(uint16_t)d5 > (int16_t)rd16(m, a4 + 0x00EEu)) /* 0x0C1978 ble */
        d5 = set_word(d5, rd16(m, a4 + 0x00EEu));    /* 0x0C197E */
    if ((int16_t)(uint16_t)d0 < (int16_t)(uint16_t)d5) /* 0x0C1982 bge */
        d0 = set_word(d0, (uint16_t)d5);             /* 0x0C1986 */
    d5 = set_word(d5, rd16(m, a4 + 0x00E8u));        /* 0x0C1988 */
    if ((int16_t)(uint16_t)d5 < (int16_t)rd16(m, a4 + 0x00F0u)) /* 0x0C198C bge */
        d5 = set_word(d5, rd16(m, a4 + 0x00F0u));    /* 0x0C1992 */
    if ((int16_t)(uint16_t)d0 > (int16_t)(uint16_t)d5) /* 0x0C1996 ble */
        d0 = set_word(d0, (uint16_t)d5);             /* 0x0C199A */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0x00A0u))); /* 0x0C199C */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0x00A4u))); /* 0x0C19A0 */
    r->d[0] = d0;
    r->d[1] = d1;
    push_long(m, r, 0x0C19AAu);                      /* 0x0C19A4 jsr 0x0BD546 */
    if (Sub_BD546(m, r, r->a[5], &d0, &d1, &a0) == 0) /* 0x0C19AA ble 0x0C19E6 */
        goto ret;

hit:                                                 /* 0x0C19AC */
    wr16(m, a4 + 0x0046u, 0u);                       /* 0x0C19AC */
    wr32(m, a4 + 0x0042u, a0);                       /* 0x0C19B0 */
    wr16(m, a4 + 0x0048u, rd16(m, a0 + 0x0018u));    /* 0x0C19B4 */
    wr16(m, a4 + 0x004Au, rd16(m, a0 + 0x001Au));    /* 0x0C19BA */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u)); /* 0x0C19C0 subq.w #1,d0 */
    if ((uint16_t)d0 != 0u)                          /* 0x0C19C2 bne 0x0C19E6 */
        goto ret;
    wr16(m, a4 + 0x0046u, 0xFFFFu);                  /* 0x0C19C4 */
    if (rd16(m, rd32(m, a4 + 0x0072u) + 0x000Cu) == 0u) /* 0x0C19CA beq 0x0C19E6 */
        goto ret;
    /* 0x0C19D6 jsr 0x0C2904 : A4+0x30 positive? */
    if ((int16_t)rd16(m, a4 + 0x0030u) >= 0)
        goto call_cedf6_ff;                          /* 0x0C19DC bne 0x0C19FA */
    /* 0x0C19DE jsr 0x0C2922 : A4+0x32 positive? */
    if ((int16_t)rd16(m, a4 + 0x0032u) >= 0)
        goto call_cedf6_1;                           /* 0x0C19E4 bne 0x0C1A0C */
    goto ret;
have_record:                                         /* 0x0C19E8 */
    a0 = rd32(m, a4 + 0x0042u);                      /* 0x0C19E8 */
    wr16(m, a4 + 0x0048u, rd16(m, a0 + 0x0018u));    /* 0x0C19EC */
    wr16(m, a4 + 0x004Au, rd16(m, a0 + 0x001Au));    /* 0x0C19F2 */
    goto ret;                                        /* 0x0C19F8 rts */
call_cedf6_ff:                                       /* 0x0C19FA */
    /* 0x0CEDF6(obj, A4+0x4, 0xFFFF) is not decompiled yet. */
    return -1;
call_cedf6_1:                                        /* 0x0C1A0C */
    /* 0x0CEDF6(obj, A4+0x4, 1) is not decompiled yet. */
    return -1;
ret:                                                 /* 0x0C19E6 / 0x0C19F8 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[5] = d5;
    r->a[0] = a0;
    r->a[7] = sp + 4u;                               /* the rts pops the PC */
    return 0;
}
