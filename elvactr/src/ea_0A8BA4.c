/* ROM 0x0A8BA4: one tick of the wounded/hit actor branch.  It ages the
 * a4+0xCC cooldown, asks 0x0049F2 whether the actor sits inside its sector,
 * and either installs the attack script (0x0A9486) or, when the actor has no
 * live input block, runs the ground test 0x00468A and finishes through the
 * shared 0x003FEA epilogue: 0x003FF4 queues the draw record into the
 * 0xA5+0x1D08 ring, 0x004856 places the 8-point box, then 0x003FBE re-arms
 * as the object's next task body. */
#include "ea_auto.h"

/* cmp2: C set when the value is outside [lo,hi] (range reversed when
 * lo > hi); Z set when it equals either bound. */
static int cmp2_w_out(int16_t v, int16_t lo, int16_t hi)
{
    return (lo <= hi) ? !(v >= lo && v <= hi) : !(v > hi || v < lo);
}

/* bftst on the bitfield window at a4+0x5E, bit n counted from the MSB. */
static int bftst_5e(const f3mem_t *m, uint32_t a4, unsigned n)
{
    return (rd8(m, a4 + 0x5Eu + (n >> 3)) >> (7u - (n & 7u))) & 1u;
}

static void bfset_5e(const f3mem_t *m, uint32_t a4, unsigned n)
{
    uint32_t a = a4 + 0x5Eu + (n >> 3);
    wr8(m, a, (uint8_t)(rd8(m, a) | (0x80u >> (n & 7u))));
}

/* ---- 0x000049F2: sector gate ------------------------------------------
 * D0.w = 0 inside the sector (and sets bit 1 of a4+0x5E), 1 when the actor
 * may move (D1.w = signed distance), -1 when the vertical test fails. */
static void h_49F2(const f3mem_t *m, uint32_t a4, uint32_t *d0io, uint32_t *d1io)
{
    uint32_t a5 = F3_A5;
    uint16_t v;

    /* 0x0049F2 move.w $4a(a4),d0 ; sub.w $1bc6(a5),d0 ; cmp2.w (8,0x11f) */
    v = (uint16_t)(rd16(m, a4 + 0x4Au) - rd16(m, a5 + 0x1BC6u));
    if (cmp2_w_out((int16_t)v, 8, 0x11F)) {
        /* 0x004A02 move.w #$ffff,d0 ; rts */
        *d0io = set_word(*d0io, 0xFFFFu);
        return;
    }
    /* 0x004A08 move.w $46(a4),d0 ; add.w $52(a4),d0 ; sub.w $1bc4(a5),d0 */
    v = (uint16_t)(rd16(m, a4 + 0x46u) + rd16(m, a4 + 0x52u));
    v = (uint16_t)(v - rd16(m, a5 + 0x1BC4u));
    /* 0x004A14 move.w d0,d1 ; sub.w #$147,d1 ; bgt $4a26 */
    {
        int16_t d1 = (int16_t)((int16_t)v - 0x0147);
        if (d1 > 0) {
            *d0io = set_word(*d0io, 1u);           /* 0x004A26 */
            *d1io = set_word(*d1io, (uint16_t)d1);
            return;
        }
        /* 0x004A1C move.w d0,d1 (d1 reloaded from D0) ; sub.w #$fff8,d1 ;
         * bge $4a2c */
        d1 = (int16_t)((int16_t)v - (int16_t)0xFFF8);
        if (d1 >= 0) {
            bfset_5e(m, a4, 1);                    /* 0x004A2C */
            *d0io = set_word(*d0io, 0u);           /* 0x004A32 clr.w d0 */
            *d1io = set_word(*d1io, (uint16_t)d1);
            return;
        }
        /* 0x004A24 neg.w d1 ; falls through to the "1" return */
        *d0io = set_word(*d0io, 1u);
        *d1io = set_word(*d1io, (uint16_t)(-(int16_t)d1));
    }
}

/* ---- 0x000BD516 / 0x000BD546: terrain + object box probe --------------
 * 0xBD516 returns 1 when the terrain bit is solid, else 0 (and 0 when the
 * probe is out of the strip range).  0xBD546 saves D0-D1 around it and,
 * with the flags of that call, either returns the caller's D0 unchanged or
 * scans the 0x41B81C object list, returning 0 / 1 / 2. */
static uint16_t probe_BD516(const f3mem_t *m, uint16_t d0w, uint16_t d1w, int *flags)
{
    uint32_t a5 = F3_A5;
    int16_t x, y;
    uint32_t a0;

    /* 0x0BD516 sub.w $998(a5),d0 ; asr.w #2,d0 ; cmp.w $99c(a5),d0 ; bcc */
    x = (int16_t)((int16_t)(d0w - rd16(m, a5 + 0x998u)) >> 2);
    if ((uint16_t)x >= rd16(m, a5 + 0x99Cu)) {
        *flags = 0;
        return 0u;                                 /* 0x0BD542 moveq #0,d0 */
    }
    /* 0x0BD522 sub.w $99a(a5),d1 ; asr.w #1,d1 ; cmp.w $99e(a5),d1 ; bcc */
    y = (int16_t)((int16_t)(d1w - rd16(m, a5 + 0x99Au)) >> 1);
    if ((uint16_t)y >= rd16(m, a5 + 0x99Eu)) {
        *flags = 0;
        return 0u;
    }
    /* 0x0BD52E ext.l d0 ; a0 = $994(a5) ; adda.w (a0,d1.w*2),a0 ;
     * bftst (a0){0:1} ; sne.b d0 ; ext.w d0 */
    a0 = rd32(m, a5 + 0x994u);
    /* adda.w (a0,d1.w*2),a0 : brief EA 0x1200 -> D1.w, scale 2 */
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0 + 2u * (uint32_t)(int32_t)y);
    a0 += (uint32_t)(int32_t)x >> 3;   /* column byte within the row */
    *flags = (rd8(m, a0) & 0x80u) != 0u;
    return (uint16_t)(*flags ? 1u : 0u);
}

static uint16_t probe_BD546(const f3mem_t *m, uint16_t d0w, uint16_t d1w)
{
    int flags;

    (void)probe_BD516(m, d0w, d1w, &flags);

    if (flags != 0)
        return d0w;                                /* 0x0BD550 bne $bd59c */
    /* 0x0BD552 tst.w $41b818 ; bmi $bd59a */
    if ((int16_t)rd16(m, 0x41B818u) < 0)
        return 0u;
    /* 0x0BD55E move.w $41b818,d2 ; lea $41b81c,a1 */
    {
        uint16_t d2 = rd16(m, 0x41B818u);

        for (;;) {
            uint32_t a0 = rd32(m, 0x41B81Cu + 4u * (uint16_t)(rd16(m, 0x41B818u) - d2));

            /* 0x0BD56C cmp2.w $34(a0),d0 ; bcs $bd57c ;
             * 0x0BD574 cmp2.w $3c(a0),d1 ; bcc $bd5a6 */
            if (!cmp2_w_out((int16_t)d0w, (int16_t)rd16(m, a0 + 0x34u),
                            (int16_t)rd16(m, a0 + 0x36u))) {
                if (!cmp2_w_out((int16_t)d1w, (int16_t)rd16(m, a0 + 0x3Cu),
                                (int16_t)rd16(m, a0 + 0x3Eu)))
                    return 1u;                     /* 0x0BD5A6 */
            }
            /* 0x0BD57C tst.l $24(a0) ; beq $bd592 ;
             * cmp2.w $24(a0),d0 ; bcs ; cmp2.w $2c(a0),d1 ; bcc $bd59e */
            if (rd32(m, a0 + 0x24u) != 0u &&
                !cmp2_w_out((int16_t)d0w, (int16_t)rd16(m, a0 + 0x24u),
                            (int16_t)rd16(m, a0 + 0x26u)) &&
                !cmp2_w_out((int16_t)d1w, (int16_t)rd16(m, a0 + 0x2Cu),
                            (int16_t)rd16(m, a0 + 0x2Eu)))
                return 2u;                         /* 0x0BD59E */
            /* 0x0BD592 dbra d2,$bd56a */
            if (d2 == 0u)
                return 0u;                         /* 0x0BD59A moveq #0,d0 */
            d2 = (uint16_t)(d2 - 1u);
        }
    }
}

/* ---- 0x0000468A: ground / wall hit test; a4+0x62 = -1, 0 or 1 --------- */
static void h_468A(const f3mem_t *m, uint32_t a4, uint32_t *d0io, uint32_t *d1io,
                   uint32_t *d2io)
{
    uint16_t d0w, d1w, d2w;
    uint32_t a0, a1 = 0u;

    /* 0x00468A movem.l d2,-(a7) */
    uint32_t d2_saved = *d2io;
    (void)d1io;
    /* 0x00468E move.w $46(a4),d0 ; move.w $4a(a4),d1 ; addq.w #1,d1 */
    d0w = rd16(m, a4 + 0x46u);
    d1w = (uint16_t)(rd16(m, a4 + 0x4Au) + 1u);
    *d0io = set_word(*d0io, d0w);
    /* 0x004698 jsr $bd546 ; 0x00469E beq $4710 ; 0x0046A2 bgt $46ae */
    d0w = probe_BD546(m, d0w, d1w);
    if (d0w == 0u)
        goto ret0;
    if ((int16_t)d0w > 0)
        a1 = 0u;                                   /* 0x0046AE movea.l a0,a1 */
    else
        d0w = 0xFFFFu;                             /* 0x0046A6 move.w #-1,d0 */
    d2w = d0w;                                     /* 0x0046B0 move.w d0,d2 */
    /* 0x0046B2 movea.l $4(a4),a0 ; the +/-1 step is refined below */
    a0 = rd32(m, a4 + 4u);
    /* 0x0046B6 move.w $2(a0),d0 ; sub.w (a0),d0 ; beq $4710 */
    d0w = (uint16_t)(rd16(m, a0 + 2u) - rd16(m, a0));
    if (d0w == 0u)
        goto ret0;
    /* 0x0046BE asr.w #1,d0 ; add.w (a0),d0 ; move.w $4(a0),d1 */
    d0w = (uint16_t)((int16_t)d0w >> 1);
    d0w = (uint16_t)(d0w + rd16(m, a0));
    d1w = rd16(m, a0 + 4u);
    /* 0x0046C6 jsr $bd546 ; beq $4710 ; bgt $46e0 */
    d0w = probe_BD546(m, d0w, d1w);
    if (d0w == 0u)
        goto ret0;
    if ((int16_t)d0w > 0) {
        /* 0x0046E0 cmpi.w #2,d0 ; beq $4710 ; tst.w d2 ; bge $4710 */
        if (d0w == 2u || (int16_t)d2w >= 0)
            goto ret0;
        a1 = a0;                                   /* 0x0046EA movea.l a0,a1 */
        /* 0x0046EC tst.w $1a(a1) ; ble $4710 */
        if ((int16_t)rd16(m, a1 + 0x1Au) <= 0)
            goto ret0;
        d0w = 1u;                                  /* 0x0046F2 */
    } else {
        /* 0x0046D4 cmpi.w #1,d2 ; beq $4710 ; tst.w d2 ; ble $4710 */
        if (d2w == 1u || (int16_t)d2w <= 0)
            goto ret0;
        /* 0x0046FE tst.w $1a(a1) ; bge $4710 */
        if ((int16_t)rd16(m, a1 + 0x1Au) >= 0)
            goto ret0;
        d0w = 0xFFFFu;                             /* 0x004704 */
    }
    wr16(m, a4 + 0x62u, d0w);                      /* 0x0046F6 / 0x004708 */
    *d0io = set_word(*d0io, d0w);
    *d2io = d2_saved;                              /* 0x004714 movem.l (a7)+,d2 */
    return;

ret0:
    /* 0x004710 move.w #0,d0 ; 0x004714 movem.l (a7)+,d2 ; rts */
    *d0io = set_word(*d0io, 0u);
    *d2io = d2_saved;
    (void)d1w;
}

/* ---- 0x00004856: place the actor's 8-point box from its offset table ---- */
static void h_4856(const f3mem_t *m, uint32_t a4, uint32_t *d0io, uint32_t *d1io)
{
    uint32_t a0 = rd32(m, a4 + 4u);                /* 0x004856 move.l $4(a4),d0 */
    uint16_t d0w, d1w;

    *d0io = a0;                                    /* the long load sets all of D0 */
    if (a0 == 0u)
        return;                                    /* 0x00485A beq $48f4 */
    /* 0x004860 tst.w $ac(a4) ; bne $4876 */
    if (rd16(m, a4 + 0xACu) == 0u) {
        wr16(m, a4 + 0xA8u, rd16(m, a4 + 0x86u));  /* 0x004866 */
        d0w = rd16(m, a4 + 0x70u);                 /* 0x00486C */
        d1w = rd16(m, a4 + 0x7Au);                 /* 0x004870 */
    } else {
        wr16(m, a4 + 0xA8u, rd16(m, a4 + 0x88u));  /* 0x004876 */
        d0w = rd16(m, a4 + 0x72u);                 /* 0x00487C */
        d1w = rd16(m, a4 + 0x7Cu);                 /* 0x004880 */
    }
    wr16(m, a0 + 0x0u, d0w);                       /* 0x004884 */
    wr16(m, a0 + 0x2u, (uint16_t)(d0w + rd16(m, a4 + 0x76u))); /* 0x004886 */
    d0w = rd16(m, a4 + 0x74u);                     /* 0x00488E */
    wr16(m, a0 + 0x4u, d0w);                       /* 0x004892 */
    wr16(m, a0 + 0x6u, (uint16_t)(d0w + rd16(m, a4 + 0x78u))); /* 0x004896 */
    wr16(m, a0 + 0x8u, d1w);                       /* 0x00489E */
    wr16(m, a0 + 0xAu, (uint16_t)(d1w + rd16(m, a4 + 0x80u))); /* 0x0048A2 */
    d0w = rd16(m, a4 + 0x7Eu);                     /* 0x0048AA */
    wr16(m, a0 + 0xCu, d0w);                       /* 0x0048AE */
    wr16(m, a0 + 0xEu, (uint16_t)(d0w + rd16(m, a4 + 0x82u))); /* 0x0048B2 */
    wr16(m, a4 + 0xAAu, rd16(m, a4 + 0x8Au));      /* 0x0048BA */
    /* 0x0048C0 move.w $46(a4),d0 ; move.w $4a(a4),d1 ;
     * add.w $52(a4),d0 ; add.w $54(a4),d1 */
    d0w = (uint16_t)(rd16(m, a4 + 0x46u) + rd16(m, a4 + 0x52u));
    d1w = (uint16_t)(rd16(m, a4 + 0x4Au) + rd16(m, a4 + 0x54u));
    /* 0x0048D0..0x0048EA: the 8 box words become their relative corners */
    wr16(m, a0 + 0x0u, (uint16_t)(rd16(m, a0 + 0x0u) + d0w));
    wr16(m, a0 + 0x2u, (uint16_t)(rd16(m, a0 + 0x2u) + d0w));
    wr16(m, a0 + 0x4u, (uint16_t)(rd16(m, a0 + 0x4u) + d1w));
    wr16(m, a0 + 0x6u, (uint16_t)(rd16(m, a0 + 0x6u) + d1w));
    wr16(m, a0 + 0x8u, (uint16_t)(rd16(m, a0 + 0x8u) + d0w));
    wr16(m, a0 + 0xAu, (uint16_t)(rd16(m, a0 + 0xAu) + d0w));
    wr16(m, a0 + 0xCu, (uint16_t)(rd16(m, a0 + 0xCu) + d1w));
    wr16(m, a0 + 0xEu, (uint16_t)(rd16(m, a0 + 0xEu) + d1w));
    wr8(m, a0 + 0x13u, 1u);                        /* 0x0048EE */
    *d0io = set_word(*d0io, d0w);
    *d1io = set_word(*d1io, d1w);
}

/* ---- 0x00003FF4: queue the draw record into the A5+0x1D08 ring ------- */
static void h_3FF4(const f3mem_t *m, uint32_t a4, uint32_t *d0io, uint32_t *d1io,
                   uint32_t *d2io, uint32_t *d3io, uint32_t *a2io)
{
    uint32_t a5 = F3_A5;
    uint32_t d0, d1, d2, d3, a1, a2;
    uint16_t d0w;

    /* 0x003FF4 tst.w $b4(a4) ; bne $406e (rts) */
    if (rd16(m, a4 + 0xB4u) != 0u)
        return;
    /* 0x003FFA move.w $46(a4),d0 ; move.w $4a(a4),d1 ;
     * add.w $52(a4),d0 ; add.w $54(a4),d1 ;
     * sub.w $1bc4(a5),d0 ; sub.w $1bc6(a5),d1 */
    d0 = set_word(*d0io, rd16(m, a4 + 0x46u));
    d1 = set_word(*d1io, rd16(m, a4 + 0x4Au));
    d0 = set_word(d0, (uint16_t)((d0 & 0xFFFFu) + rd16(m, a4 + 0x52u)));
    d1 = set_word(d1, (uint16_t)((d1 & 0xFFFFu) + rd16(m, a4 + 0x54u)));
    d0 = set_word(d0, (uint16_t)((d0 & 0xFFFFu) - rd16(m, a5 + 0x1BC4u)));
    d1 = set_word(d1, (uint16_t)((d1 & 0xFFFFu) - rd16(m, a5 + 0x1BC6u)));
    /* 0x004012 movea.l $8(a4),a1 ; d3 = ((a1)+1)<<4 ; d2 = ((a1+2)+1)<<3 */
    a1 = rd32(m, a4 + 8u);
    d3 = set_word(*d3io, rd16(m, a1));
    d2 = set_word(*d2io, rd16(m, a1 + 2u));
    d2 = set_word(d2, (uint16_t)((d2 & 0xFFFFu) + 1u));
    d3 = set_word(d3, (uint16_t)((d3 & 0xFFFFu) + 1u));
    d2 = set_word(d2, (uint16_t)((d2 & 0xFFFFu) << 3));
    d3 = set_word(d3, (uint16_t)((d3 & 0xFFFFu) << 4));
    d0 = set_word(d0, (uint16_t)((d0 & 0xFFFFu) - (d2 & 0xFFFFu)));
    d1 = set_word(d1, (uint16_t)((d1 & 0xFFFFu) - (d3 & 0xFFFFu)));
    /* 0x004028 move.w $4e(a4),d2 ; andi.w #7,d2 */
    d2 = set_word(d2, (uint16_t)(rd16(m, a4 + 0x4Eu) & 7u));
    /* 0x004030 lea $1cf0(a5),a2 ; lea (a2,d2.w*2),a2 ; tst.w (a2) */
    a2 = a5 + 0x1CF0u + 2u * (uint16_t)(d2 & 0xFFFFu);
    if (rd16(m, a2) != 0u) {
        /* 0x00403C subq.w #1,(a2) */
        wr16(m, a2, (uint16_t)(rd16(m, a2) - 1u));
        /* 0x00403E lea $1d00(a5),a2 ; lea (a2,d2.w*4),a2 ;
         * 0x004046 move.l a2,d2 ; 0x004048 movea.l (a2),a2 */
        a2 = a5 + 0x1D00u + 4u * (uint16_t)(d2 & 0xFFFFu);
        d2 = a2;
        a2 = rd32(m, a2);
        /* 0x00404A..0x004062: 7 words + the long A1, predecrementing */
        a2 -= 2u; wr16(m, a2, 0x0000u);
        a2 -= 2u; wr16(m, a2, rd16(m, a4 + 0xACu));
        a2 -= 2u; wr16(m, a2, rd16(m, a4 + 0x96u));
        a2 -= 2u; wr16(m, a2, (uint16_t)d1);
        a2 -= 2u; wr16(m, a2, (uint16_t)d0);
        a2 -= 2u; wr16(m, a2, 0x0000u);
        a2 -= 2u; wr16(m, a2, 0x0000u);
        a2 -= 4u; wr32(m, a2, a1);
        wr32(m, d2, a2);                           /* 0x004064 move.l a2,(d2.l) */
        *a2io = a2;
    }
    /* 0x004068 jsr $4856 */
    h_4856(m, a4, d0io, d1io);
    (void)d0w;
    *d2io = d2;
    *d3io = d3;
}

int Ea_0A8BA4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t a0, a2 = r->a[2];

    /* 0x0A8BA4 move.w $cc(a4),d0 ; beq $a8bae ; subq.w #1,$cc(a4) */
    if (rd16(m, a4 + 0xCCu) != 0u) {
        d0 = set_word(d0, rd16(m, a4 + 0xCCu));
        wr16(m, a4 + 0xCCu, (uint16_t)(rd16(m, a4 + 0xCCu) - 1u));
        d0 = set_word(d0, (uint16_t)((d0 & 0xFFFFu) - 1u));
    } else {
        d0 = set_word(d0, 0u);
    }

    /* 0x0A8BAE jsr $49f2 ; tst.w d0 ; beq $a8be6 ; blt $a8bbe ;
     * 0x0A8BB8 cmpi.w #$20,d1 ; blt $a8be6 */
    h_49F2(m, a4, &d0, &d1);
    if ((uint16_t)d0 == 0u)
        goto L8BE6;
    if ((int16_t)(uint16_t)d0 < 0)
        goto L8BBE;
    if ((int16_t)(uint16_t)d1 < 0x20)
        goto L8BE6;

L8BBE:
    /* 0x0A8BBE bftst $5e(a4){11:1} ; beq $a8bce ; bftst {1:1} ; beq $a8be6 */
    if (bftst_5e(m, a4, 11) && !bftst_5e(m, a4, 1))
        goto L8BE6;
    /* 0x0A8BCE move.l #$a9486,$1c(a4) ; lea $34(a4),a0 ;
     * move.l a0,$20(a4) ; clr.w $44(a4) ; bra $a8d4a */
    wr32(m, a4 + 0x1Cu, 0x000A9486u);
    a0 = a4 + 0x34u;
    wr32(m, a4 + 0x20u, a0);
    wr16(m, a4 + 0x44u, 0u);
    goto L8D4A;

L8BE6:
    /* 0x0A8BE6 tst.w $56(a4) ; blt $a8d4a */
    if ((int16_t)rd16(m, a4 + 0x56u) < 0)
        goto L8D4A;
    /* 0x0A8BEE movea.l $4(a4),a0 ; tst.w $10(a0) ; beq $a8cf2 */
    a0 = rd32(m, a4 + 4u);
    if (rd16(m, a0 + 0x10u) == 0u)
        goto L8CF2;

    /* 0x0A8BFA subq.w #1,$56(a4) ; btst.b #4,$10(a0) ; beq ; clr.w $56(a4) ;
     * 0x0A8C0A bsr $a826a ; btst.b #6,$10(a0) ; bne $a8d1e */
    wr16(m, a4 + 0x56u, (uint16_t)(rd16(m, a4 + 0x56u) - 1u));
    if ((rd8(m, a0 + 0x10u) & 0x10u) != 0u)
        wr16(m, a4 + 0x56u, 0u);
    wr16(m, a4 + 0x54u, 0x000Au);                  /* 0x0A826A bsr */
    if ((rd8(m, a0 + 0x10u) & 0x40u) != 0u)
        goto L8D1E;

    /* The live-input attack path (0x0A8C18..0x0A8CA2) stages sounds through
     * 0x004A50/0x004CB0/0x004CFC/0x004DB4 and spawns actors through
     * 0x0043B2 -> 0x00393C -> the 0x0A6352 table, none of which is
     * decompiled yet. */
    { r->a[7] = sp + 4u; return -1; }

L8CF2:
    /* 0x0A8CF2 jsr $468a ; tst.w d0 ; beq $a8d1a */
    h_468A(m, a4, &d0, &d1, &d2);
    if ((uint16_t)d0 == 0u)
        goto L8D1A;
    /* 0x0A8CFA move.w #$1,$cc(a4) ; jsr $4b6e ; move.l #$a9448,$1c(a4) */
    wr16(m, a4 + 0xCCu, 1u);
    /* 0x0A8D00 jsr $4b6e -> 0x004A50 + spawn chain, not decompiled yet. */
    { r->a[7] = sp + 4u; return -1; }

L8D1A:
    /* 0x0A8D1A jmp $3fea */
    goto L8D4A;

L8D1E:
    /* 0x0A8D1E jsr $4a50 ; move.w #$ffff,$56(a4) ;
     * move.w #$36,-(a7) ; jsr $2f3a ; move.l #$ab9f2,$1c(a4)
     * (0x004A50 and the 0x002F3A sound queue are not decompiled yet) */
    { r->a[7] = sp + 4u; return -1; }

L8D4A:
    /* 0x0A8D4A jmp $3fea: bsr $3ff4 ; bra $3fb6 */
    h_3FF4(m, a4, &d0, &d1, &d2, &d3, &a2);
    /* 0x003FB6 lea $3fbe(pc),a6 ; move.l a6,(a4) ; rts */
    wr32(m, a4 + 0x00u, 0x00003FBEu);

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->a[0] = rd32(m, a4 + 4u);                    /* 0x00485E movea.l d0,a0 */
    r->a[1] = rd32(m, a4 + 8u);                    /* 0x004012 movea.l $8(a4),a1 */
    r->a[2] = a2;                                  /* 0x004048/0x004062 predecrements */
    r->a[6] = 0x00003FBEu;                         /* 0x003FB6 lea $3fbe(pc),a6 */
    r->a[7] = sp + 4u;
    return 0;
}
