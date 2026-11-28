/* ROM 0x12FF6C: playfield column filler ("wipe") -- walks a ROM word table
 * (y, height, count) at A5+0x34D4 and paints `count` columns, one per frame
 * step built from height/count, into the two framebuffer planes at
 * 0x624630/0x625430, or backwards into 0x6247D0/0x6255D0 when the A5-0x7EC2
 * flag is set; a count with bit15 set uses the A5-0x785E RNG (0x1338) per
 * column instead of a linear step. A negative first word ends the table. */
#include "ea_auto.h"

/* 0x12FFB2 / 0x130074 divs.l d5,d6 (extension word 0x6806: bit10 = 0, so a
 * 32/32 divide, quotient in D6 and the remainder NOT stored).  D6 is left
 * unchanged on overflow or on a zero divisor. */
static uint32_t divs32(uint32_t num, uint32_t den)
{
    int64_t q;

    if (den == 0u)
        return num;
    q = (int64_t)(int32_t)num / (int64_t)(int32_t)den;  /* truncates toward 0 */
    if (q > 2147483647LL || q < -2147483648LL)
        return num;                                     /* V set, Dn unchanged */
    return (uint32_t)(int32_t)q;
}

/* 0x12FFC8..0x12FFD8 and 0x13008A..0x130098: the two plane words of one
 * column, then the store.  fwd advances A1/A2, otherwise they predecrement. */
static void emit_col(const f3mem_t *m, uint32_t *a1, uint32_t *a2, uint16_t y,
                     int fwd)
{
    uint16_t w0 = (uint16_t)((uint16_t)(y << 5) & 0x2000u);  /* lsl.w #5 ; andi.w #$2000 */
    uint16_t w1 = (uint16_t)((uint16_t)(y << 8) & 0xFF00u);  /* lsl.w #8 ; andi.w #$ff00 */

    if (fwd) {
        wr16(m, *a1, w0);
        *a1 += 2u;
        wr16(m, *a2, w1);
        *a2 += 2u;
    } else {
        *a1 -= 2u;
        wr16(m, *a1, w0);
        *a2 -= 2u;
        wr16(m, *a2, w1);
    }
}

int Ea_12FF6C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t d4 = r->d[4], d5 = r->d[5], d6 = r->d[6], d7 = r->d[7];
    uint32_t a0 = r->a[0], a1 = r->a[1], a2 = r->a[2], a3 = r->a[3];
    uint32_t fr;
    uint16_t cnt;
    int rev;

    /* 0x12FF6C tst.w -$7ec2(a5) ; 0x12FF70 bne.w 0x13001c */
    rev = rd16(m, a5 - 0x7EC2u) != 0u;

    if (!rev) {
        /* ---------- 0x12FF74: forward fill, A1 = 0x624630 A2 = 0x625430 --- */
        fr = sp - 44u;                       /* 0x12FF74 movem.l d0-d7/a0-a2,-(a7) */
        r->a[7] = fr;
        wr32(m, fr + 0u, d0);
        wr32(m, fr + 4u, d1);
        wr32(m, fr + 8u, d2);
        wr32(m, fr + 12u, d3);
        wr32(m, fr + 16u, d4);
        wr32(m, fr + 20u, d5);
        wr32(m, fr + 24u, d6);
        wr32(m, fr + 28u, d7);
        wr32(m, fr + 32u, a0);
        wr32(m, fr + 36u, a1);
        wr32(m, fr + 40u, a2);

        cnt = (uint16_t)(rd16(m, a5 + 0x34D2u) - 1u);   /* 0x12FF78 subq.w #1 */
        wr16(m, a5 + 0x34D2u, cnt);
        if ((int16_t)cnt > 0) {
            /* 0x12FF7C bgt.w 0x12ffe6: this frame only re-arms, no columns */
        } else {
            wr16(m, a5 + 0x34D2u, 1u);                  /* 0x12FF80 move.w #1 */
            a0 = rd32(m, a5 + 0x34D4u);                 /* 0x12FF86 movea.l */
            a1 = 0x624630u;                             /* 0x12FF8A lea */
            a2 = 0x625430u;                             /* 0x12FF90 lea */

            for (;;) {
                uint16_t y;
                int rnd;

                d7 = (d7 & 0xFFFF0000u) | rd16(m, a0);   /* 0x12FF96 move.w (a0)+,d7 */
                a0 += 2u;
                if ((d7 & 0x8000u) != 0u)                /* 0x12FF98 bmi.w 0x12ffe2 */
                    break;                               /* end of table */
                d7 = (d7 & 0xFFFF0000u) | (uint16_t)(d7 + 0x2Fu); /* 0x12FF9C */
                d7 = (d7 << 16) | (d7 >> 16);            /* 0x12FFA0 swap d7 */
                d7 = d7 & 0xFFFF0000u;                   /* 0x12FFA2 clr.w d7 */
                d6 = (d6 & 0xFFFF0000u) | rd16(m, a0);   /* 0x12FFA4 move.w (a0)+,d6 */
                a0 += 2u;
                d6 = (d6 << 16) | (d6 >> 16);            /* 0x12FFA6 swap d6 */
                d6 = d6 & 0xFFFF0000u;                   /* 0x12FFA8 clr.w d6 */
                d5 = (d5 & 0xFFFF0000u) | rd16(m, a0);   /* 0x12FFAA move.w (a0)+,d5 */
                a0 += 2u;
                rnd = (d5 & 0x8000u) != 0u;              /* 0x12FFAC bmi.w 0x12ffec */
                if (!rnd) {
                    d5 = (d5 & 0xFFFF0000u) | (uint16_t)(int16_t)(uint16_t)d5; /* 0x12FFB0 ext.l d5 */
                    d6 = divs32(d6, d5);                 /* 0x12FFB2 divs.l d5,d6 */
                } else {
                    d5 = (d5 & 0xFFFF0000u) | (uint16_t)(-(int16_t)(uint16_t)d5); /* 0x12FFEC neg.w d5 */
                }
                d5 = (d5 & 0xFFFF0000u) | (uint16_t)(d5 - 1u); /* 0x12FFB6 / 0x12FFEE subq.w #1 */

                do {
                    if (rnd) {
                        /* 0x12FFF0 jsr 0x1338: D0 = swapped seed, one call per column */
                        r->d[0] = d0;
                        push_long(m, r, 0x12FFF6u);
                        Ea_001338(m, r);
                        d0 = r->d[0];
                        d0 = (d0 & 0xFFFF0000u) | (d0 & 3u);          /* 0x12FFF6 andi.w #3 */
                        d1 = (d7 << 16) | (d7 >> 16);                 /* 0x12FFFA swap d1 */
                        d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 - (d0 & 0xFFFFu)); /* 0x12FFFE sub.w d0,d1 */
                        y = (uint16_t)d1;                             /* 0x130000 move.w d1,d0 */
                        d0 = (d0 & 0xFFFF0000u) | y;
                    } else {
                        d1 = (d7 << 16) | (d7 >> 16);                 /* 0x12FFB8 move.l d7,d1 ; swap */
                        y = (uint16_t)d1;
                        if (y >= 0x16Fu)                              /* 0x12FFBC cmpi.w #$16f */
                            y = 0x1C0u;                               /* 0x12FFC0 bcs / 0x12FFC4 */
                        d0 = (d0 & 0xFFFF0000u) | y;                  /* 0x12FFC8 move.w d1,d0 */
                    }
                    emit_col(m, &a1, &a2, y, 1);                      /* 0x12FFCA..0x12FFD8 */
                    d7 += d6;                                         /* 0x12FFDA add.l d6,d7 */
                    d5 = (d5 & 0xFFFF0000u) | (uint16_t)(d5 - 1u);    /* 0x12FFDC dbra d5 */
                } while ((uint16_t)d5 != 0xFFFFu);

                /* 0x12FFE0 bra.b 0x12ff96: next (y,height,count) group */
            }
            wr32(m, a5 + 0x34D4u, a0);                  /* 0x12FFE2 move.l a0,$34d4(a5) */
        }
    } else {
        /* ---------- 0x13001C: reverse fill, A1 = 0x6247D0 A2 = 0x6255D0 --- */
        fr = sp - 48u;                          /* 0x13001C movem.l d0-d7/a0-a3,-(a7) */
        r->a[7] = fr;
        wr32(m, fr + 0u, d0);
        wr32(m, fr + 4u, d1);
        wr32(m, fr + 8u, d2);
        wr32(m, fr + 12u, d3);
        wr32(m, fr + 16u, d4);
        wr32(m, fr + 20u, d5);
        wr32(m, fr + 24u, d6);
        wr32(m, fr + 28u, d7);
        wr32(m, fr + 32u, a0);
        wr32(m, fr + 36u, a1);
        wr32(m, fr + 40u, a2);
        wr32(m, fr + 44u, a3);

        cnt = (uint16_t)(rd16(m, a5 + 0x34D2u) - 1u);   /* 0x130020 subq.w #1 */
        wr16(m, a5 + 0x34D2u, cnt);
        if ((int16_t)cnt > 0) {
            /* 0x130024 bgt.w 0x1300a8: this frame only re-arms */
        } else {
            wr16(m, a5 + 0x34D2u, 1u);                  /* 0x130028 move.w #1 */
            a0 = rd32(m, a5 + 0x34D4u);                 /* 0x13002E movea.l */
            a1 = 0x6247D0u;                             /* 0x130032 lea */
            a2 = 0x6255D0u;                             /* 0x130038 lea */

            while ((rd16(m, a0) & 0x8000u) == 0u)       /* 0x13003E tst.w (a0) / bmi */
                a0 += 6u;                               /* 0x130044 addq.w #6,a0 */

            a3 = a0 + 2u;                               /* 0x130048 lea 2(a0),a3 */

            for (;;) {
                uint16_t y;
                int rnd;

                a0 -= 2u;
                d5 = (d5 & 0xFFFF0000u) | rd16(m, a0);  /* 0x13004C move.w -(a0),d5 */
                if ((uint16_t)d5 == 0xFFFFu) {          /* 0x13004E cmpi.w #$ffff */
                    wr32(m, a5 + 0x34D4u, a3);          /* 0x130052 beq 0x1300a4 */
                    break;                              /* end of table */
                }
                a0 -= 2u;
                d6 = (d6 & 0xFFFF0000u) | rd16(m, a0);  /* 0x130056 move.w -(a0),d6 */
                d7 = (d7 & 0xFFFF0000u) | 0x0140u;      /* 0x130058 move.w #$140,d7 */
                d7 = (d7 & 0xFFFF0000u) | (uint16_t)(d7 - (d6 & 0xFFFFu)); /* 0x13005C */
                a0 -= 2u;
                d7 = (d7 & 0xFFFF0000u) | (uint16_t)(d7 - rd16(m, a0)); /* 0x13005E sub.w -(a0),d7 */
                d7 = (d7 & 0xFFFF0000u) | (uint16_t)(d7 + 0x30u);       /* 0x130060 addi.w #$30 */
                d7 = (d7 << 16) | (d7 >> 16);            /* 0x130064 swap d7 */
                d7 = d7 & 0xFFFF0000u;                   /* 0x130066 clr.w d7 */
                d6 = (d6 << 16) | (d6 >> 16);            /* 0x130068 swap d6 */
                d6 = d6 & 0xFFFF0000u;                   /* 0x13006A clr.w d6 */
                rnd = (d5 & 0x8000u) != 0u;              /* 0x13006C tst.w d5 / bmi */
                if (!rnd) {
                    d5 = (d5 & 0xFFFF0000u) | (uint16_t)(int16_t)(uint16_t)d5; /* 0x130072 ext.l */
                    d6 = divs32(d6, d5);                 /* 0x130074 divs.l d5,d6 */
                } else {
                    d5 = (d5 & 0xFFFF0000u) | (uint16_t)(-(int16_t)(uint16_t)d5); /* 0x1300AE neg.w */
                }
                d5 = (d5 & 0xFFFF0000u) | (uint16_t)(d5 - 1u); /* 0x130078 / 0x1300B0 subq.w #1 */

                do {
                    if (rnd) {
                        /* 0x1300B2 jsr 0x1338: one RNG call per column */
                        r->d[0] = d0;
                        push_long(m, r, 0x1300B8u);
                        Ea_001338(m, r);
                        d0 = r->d[0];
                        d0 = (d0 & 0xFFFF0000u) | (d0 & 3u);          /* 0x1300B8 andi.w #3 */
                        d1 = (d7 << 16) | (d7 >> 16);                 /* 0x1300BC swap d1 */
                        d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 - (d0 & 0xFFFFu)); /* 0x1300C0 */
                        y = (uint16_t)d1;                             /* 0x1300C2 */
                        d0 = (d0 & 0xFFFF0000u) | y;
                    } else {
                        d1 = (d7 << 16) | (d7 >> 16);                 /* 0x13007A move.l d7,d1 ; swap */
                        y = (uint16_t)d1;
                        if (y >= 0x16Fu)                              /* 0x13007E cmpi.w #$16f */
                            y = 0x1C0u;                               /* 0x130082 bcs / 0x130086 */
                        d0 = (d0 & 0xFFFF0000u) | y;                  /* 0x13008A move.w d1,d0 */
                    }
                    emit_col(m, &a1, &a2, y, 0);                      /* 0x13008C..0x13009A */
                    d7 += d6;                                         /* 0x13009C add.l d6,d7 */
                    d5 = (d5 & 0xFFFF0000u) | (uint16_t)(d5 - 1u);    /* 0x13009E dbra d5 */
                } while ((uint16_t)d5 != 0xFFFFu);

                /* 0x1300A2 bra.b 0x13004c: next group, pointer moves down */
            }
        }
    }

    /* 0x12FFE6 / 0x1300A8: movem.l (a7)+,<same list> then 0x12FFEA / 0x1300AC rts */
    d0 = rd32(m, fr + 0u);
    d1 = rd32(m, fr + 4u);
    d2 = rd32(m, fr + 8u);
    d3 = rd32(m, fr + 12u);
    d4 = rd32(m, fr + 16u);
    d5 = rd32(m, fr + 20u);
    d6 = rd32(m, fr + 24u);
    d7 = rd32(m, fr + 28u);
    a0 = rd32(m, fr + 32u);
    a1 = rd32(m, fr + 36u);
    a2 = rd32(m, fr + 40u);
    if (rev) {
        a3 = rd32(m, fr + 44u);
        r->a[7] = fr + 48u + 4u;
    } else {
        r->a[7] = fr + 44u + 4u;
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[5] = d5;
    r->d[6] = d6;
    r->d[7] = d7;
    r->a[0] = a0;
    r->a[1] = a1;
    r->a[2] = a2;
    r->a[3] = a3;
    return 0;
}
