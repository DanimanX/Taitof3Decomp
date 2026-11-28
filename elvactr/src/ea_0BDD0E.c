/* ROM 0x0BDD0E: pick the range record under a pushed position. It turns the
 * pushed position (word at SP+4 scaled by A5+0x998, words at SP+6/SP+8 scaled
 * by A5+0x99A) into a strip index, reads that strip's row via the offset table
 * at A5+0x990 (word offsets added to its own base) and walks the row's key
 * table for the key covering the pushed word, then scans the object list at
 * 0x41B81C (count 0x41B818, bit 15 = no list) keeping the best {lo,hi} record
 * pointer. The results replace the two pushed arguments: word (SP+4) = found
 * key, long (SP+6) = record pointer; the movem epilogue restores everything
 * but SP (= entry SP + 4). */
#include "ea_auto.h"

/* signed 16-bit view, the width every .w compare/load in the routine uses */
static int16_t s16(uint16_t v)
{
    return (int16_t)v;
}

/* asr.w #n on a 16-bit operand: arithmetic, rounds towards -inf. */
static uint16_t asr16(uint16_t v, unsigned n)
{
    int16_t s = (int16_t)v;
    return s >= 0 ? (uint16_t)(s >> n) : (uint16_t)~((uint16_t)(~s) >> n);
}

/* One object slot: D4 <= key_lo, D0 > key_lo, D1 <= limit_hi, D2 >= limit_lo
 * (0x0BDDCC..0x0BDDE2 for the +0x24 box, 0x0BDDEC..0x0BDE02 for +0x34).
 * `lo` is the key word, +0x0A the D1 limit and +0x08 the D2 limit. */
static int box_hit(const f3mem_t *m, uint32_t base, uint32_t lo, uint16_t d0w,
                   uint16_t d1w, uint16_t d2w, uint16_t d4w)
{
    return s16(d4w) <= s16(rd16(m, base + lo)) &&
           s16(d0w) >  s16(rd16(m, base + lo)) &&
           s16(d1w) <= s16(rd16(m, base + lo + 0x0Au)) &&
           s16(d2w) >= s16(rd16(m, base + lo + 0x08u));
}

int Ea_0BDD0E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t S = r->a[7];      /* return address; the pushed words are at S+4.. */
    uint32_t sp = S - 32u;     /* 0x0BDD0E movem.l d0-d4/a0-a2,-(a7) */
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3], d4 = r->d[4];
    uint32_t a0 = r->a[0], a1 = r->a[1], a2 = r->a[2];
    uint16_t w;

    wr32(m, sp +  0u, d0);
    wr32(m, sp +  4u, d1);
    wr32(m, sp +  8u, d2);
    wr32(m, sp + 12u, d3);
    wr32(m, sp + 16u, d4);
    wr32(m, sp + 20u, a0);
    wr32(m, sp + 24u, a1);
    wr32(m, sp + 28u, a2);

    d3 = 0xFFFFFFFFu;                                           /* 0x0BDD12 moveq #$ff,d3 */
    d0 = (d0 & 0xFFFF0000u) |                                   /* 0x0BDD14 move.w $24(a7),d0 */
         (uint16_t)(rd16(m, S + 4u) - rd16(m, a5 + 0x998u));    /* 0x0BDD18 sub.w $998(a5),d0 */
    d0 = (d0 & 0xFFFF0000u) | asr16((uint16_t)d0, 2);           /* 0x0BDD1C asr.w #2,d0 */

    if ((uint16_t)d0 >= rd16(m, a5 + 0x99Cu)) {                 /* 0x0BDD1E bcc.w 0x0BDE7E */
        d0 = (d0 & 0xFFFF0000u) | 0x7FFFu;                      /* 0x0BDE7E */
        a0 = 0x000BD5AEu;                                       /* 0x0BDE82 */
    } else {
        /* 0x0BDD26: D2 from the pushed SP+8 word, biased, halved, clamped */
        w = (uint16_t)(rd16(m, S + 8u) - rd16(m, a5 + 0x99Au));
        if (s16(w) < 0)
            w = 0u;                                             /* 0x0BDD30 moveq #0,d2 */
        else {
            w = asr16(w, 1);                                    /* 0x0BDD34 */
            if (w >= rd16(m, a5 + 0x99Eu))                      /* 0x0BDD36 bcs */
                w = (uint16_t)(rd16(m, a5 + 0x99Eu) - 1u);      /* 0x0BDD3C/0x0BDD40 */
        }
        d2 = (d2 & 0xFFFF0000u) | w;

        /* 0x0BDD42: D1 from the pushed SP+6 word, same bias/halving/clamp */
        w = (uint16_t)(rd16(m, S + 6u) - rd16(m, a5 + 0x99Au));
        if (s16(w) < 0)
            w = 0u;                                             /* 0x0BDD4C moveq #0,d1 */
        else {
            w = asr16(w, 1);                                    /* 0x0BDD50 */
            if (w >= rd16(m, a5 + 0x99Eu))                      /* 0x0BDD52 bcs */
                w = (uint16_t)(rd16(m, a5 + 0x99Eu) - 1u);      /* 0x0BDD58/0x0BDD5C */
        }
        d1 = (d1 & 0xFFFF0000u) | w;

        a1 = rd32(m, a5 + 0x990u);                              /* 0x0BDD5E movea.l $990(a5),a1 */
        d1 = (d1 & 0xFFFF0000u) |                               /* 0x0BDD62 move.w (a1,d1.w*2),d1 */
             rd16(m, a1 + (uint32_t)(int32_t)(s16((uint16_t)d1) * 2));
        d2 = (d2 & 0xFFFF0000u) |                               /* 0x0BDD66 move.w (a1,d2.w*2),d2 */
             rd16(m, a1 + (uint32_t)(int32_t)(s16((uint16_t)d2) * 2));
        d2 = (d2 & 0xFFFF0000u) |                               /* 0x0BDD6A sub.w d1,d2 */
             (uint16_t)((uint16_t)d2 - (uint16_t)d1);
        d4 = (d4 & 0xFFFF0000u) | rd16(m, S + 4u);              /* 0x0BDD6C move.w $24(a7),d4 */
        d0 = (d0 & 0xFFFF0000u) | 0x7FFFu;                      /* 0x0BDD70 move.w #$7fff,d0 */
        a0 = 0x000BD5AEu;                                       /* 0x0BDD74 movea.l #$bd5ae,a0 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)(-(uint16_t)d1);    /* 0x0BDD7A neg.w d1 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 - 1u);/* 0x0BDD7C subq.w #1,d1 */
        d1 = (d1 & 0xFFFF0000u) |                               /* 0x0BDD7E add.w d1,d1 */
             (uint16_t)((uint16_t)d1 + (uint16_t)d1);

        /* 0x0BDD80..0x0BDD9A: one row per iteration; D1 walks the offset table
         * downwards by two bytes, so the row index rises by one each time. */
        for (;;) {
            /* 0x0BDD80/0x0BDD82 movea.l a1,a2 ; adda.w (a1,d1.w),a2 -- ADDA
             * adds the memory WORD at A1+D1 (the row's offset from the base). */
            a2 = a1 + (uint32_t)(int32_t)
                      s16(rd16(m, a1 + (uint32_t)(int32_t)s16((uint16_t)d1)));
            a2 -= 4u;                                           /* 0x0BDD86 subq.w #4,a2 */
            do {
                a2 += 4u;                                       /* 0x0BDD88 addq.w #4,a2 */
            } while (s16((uint16_t)d4) > s16(rd16(m, a2)));     /* 0x0BDD8A bgt */
            if (s16((uint16_t)d0) > s16(rd16(m, a2))) {          /* 0x0BDD8E ble */
                d0 = (d0 & 0xFFFF0000u) | rd16(m, a2);          /* 0x0BDD92 move.w (a2)+,d0 */
                a2 += 2u;
                a0 = a2;                                        /* 0x0BDD94 lea (a2),a0 */
                a0 += (uint32_t)(int32_t)s16(rd16(m, a2));      /* 0x0BDD96 adda.w (a2),a0 */
            }
            d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 - 2u); /* 0x0BDD98 */
            d2 = (d2 & 0xFFFF0000u) | (uint16_t)((uint16_t)d2 - 1u); /* 0x0BDD9A dbra d2 */
            if ((uint16_t)d2 == 0xFFFFu)
                break;
        }
    }

    /* 0x0BDD9E: bit 15 of the list length means "no list to scan" */
    if (s16(rd16(m, 0x41B818u)) < 0)
        goto store;                                             /* 0x0BDDA4 bmi.w 0x0BDE70 */

    a2 = 0x0041B81Cu;                                           /* 0x0BDDA8 lea $41b81c.l,a2 */
    d1 = (d1 & 0xFFFF0000u) | rd16(m, S + 6u);                  /* 0x0BDDAE */
    d2 = (d2 & 0xFFFF0000u) | rd16(m, S + 8u);                  /* 0x0BDDB2 */
    d4 = (d4 & 0xFFFF0000u) | rd16(m, S + 4u);                  /* 0x0BDDB6 */

    if ((uint16_t)d3 == 0u) {
        /* 0x0BDE1E..0x0BDE6C: alternate box order driven by D1/D2 vs D0/D4.
         * D3 is always -1 here, so the ROM never reaches this from 0x0BDD0E;
         * kept for fidelity. */
        d3 = (d3 & 0xFFFF0000u) | rd16(m, 0x41B818u);           /* 0x0BDE1E */
        for (;;) {
            a1 = rd32(m, a2);                                   /* 0x0BDE24 movea.l (a2)+,a1 */
            a2 += 4u;
            if (rd32(m, a1 + 0x24u) == 0u)                      /* 0x0BDE26 beq */
                goto alt_second;
            if (s16((uint16_t)d2) < s16(rd16(m, a1 + 0x24u)))   /* 0x0BDE2C blt */
                goto alt_second;
            if (s16((uint16_t)d1) > s16(rd16(m, a1 + 0x26u)))   /* 0x0BDE32 bgt */
                goto alt_second;
            if (s16((uint16_t)d0) <= s16(rd16(m, a1 + 0x2Cu)))  /* 0x0BDE38 ble */
                goto alt_second;
            if (s16((uint16_t)d4) > s16(rd16(m, a1 + 0x2Cu)))   /* 0x0BDE3E bgt */
                goto alt_second;
            d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 0x2Cu);      /* 0x0BDE44 */
            a0 = a1 + 0x24u;                                    /* 0x0BDE48 */
        alt_second:
            if (s16((uint16_t)d2) < s16(rd16(m, a1 + 0x34u)))   /* 0x0BDE4C blt */
                goto alt_next;
            if (s16((uint16_t)d1) > s16(rd16(m, a1 + 0x36u)))   /* 0x0BDE52 bgt */
                goto alt_next;
            if (s16((uint16_t)d0) <= s16(rd16(m, a1 + 0x3Cu)))  /* 0x0BDE58 ble */
                goto alt_next;
            if (s16((uint16_t)d4) > s16(rd16(m, a1 + 0x3Cu)))   /* 0x0BDE5E bgt */
                goto alt_next;
            d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 0x3Cu);      /* 0x0BDE64 */
            a0 = a1 + 0x34u;                                    /* 0x0BDE68 */
        alt_next:
            d3 = (d3 & 0xFFFF0000u) | (uint16_t)((uint16_t)d3 - 1u); /* 0x0BDE6C dbra d3 */
            if ((uint16_t)d3 == 0xFFFFu)
                break;
        }
    } else {
        /* 0x0BDDBE..0x0BDE0C: scan the live objects, both boxes each */
        d3 = (d3 & 0xFFFF0000u) | rd16(m, 0x41B818u);           /* 0x0BDDBE */
        for (;;) {
            a1 = rd32(m, a2);                                   /* 0x0BDDC4 movea.l (a2)+,a1 */
            a2 += 4u;
            if (rd32(m, a1 + 0x24u) != 0u &&                    /* 0x0BDDC6 tst.l / beq */
                box_hit(m, a1, 0x24u, (uint16_t)d0, (uint16_t)d1,
                        (uint16_t)d2, (uint16_t)d4)) {
                d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 0x24u);  /* 0x0BDDE4 */
                a0 = a1 + 0x2Cu;                                /* 0x0BDDE8 lea $2c(a1),a0 */
            }
            if (box_hit(m, a1, 0x34u, (uint16_t)d0, (uint16_t)d1,
                        (uint16_t)d2, (uint16_t)d4)) {
                d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 0x34u);  /* 0x0BDE04 */
                a0 = a1 + 0x3Cu;                                /* 0x0BDE08 lea $3c(a1),a0 */
            }
            d3 = (d3 & 0xFFFF0000u) | (uint16_t)((uint16_t)d3 - 1u); /* 0x0BDE0C dbra d3 */
            if ((uint16_t)d3 == 0xFFFFu)
                break;
        }
    }

store:
    wr16(m, S + 4u, (uint16_t)d0);                              /* 0x0BDE10/0x0BDE70 */
    wr32(m, S + 6u, a0);                                        /* 0x0BDE14/0x0BDE74 */

    /* 0x0BDE18/0x0BDE78 movem.l (a7)+,d0-d4/a0-a2 */
    r->d[0] = rd32(m, sp +  0u);
    r->d[1] = rd32(m, sp +  4u);
    r->d[2] = rd32(m, sp +  8u);
    r->d[3] = rd32(m, sp + 12u);
    r->d[4] = rd32(m, sp + 16u);
    r->a[0] = rd32(m, sp + 20u);
    r->a[1] = rd32(m, sp + 24u);
    r->a[2] = rd32(m, sp + 28u);
    r->a[7] = S + 4u;                                           /* 0x0BDE1C rts */
    return 0;
}
