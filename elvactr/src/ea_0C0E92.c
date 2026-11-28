/* ROM 0x0C0E92: per-frame ground handling for the actor at A4 - clamp its
 * candidate surface box, probe the map under one or two points (0x0BD9F2),
 * and snap A4+0xA4 onto the hit surface, re-deriving the ground records at
 * A4+0x42/0x46/0x66/0x86.  D7 carries the probe's -4 (or -$C6.w-5) bias.
 *
 * Layout of the ROM code:
 *   0C0E92..0C0EA4  D7 = (long A4+0xC6 > 0) ? -(A4+0xC6).w - 1 : 0, then -4
 *   0C0EA6..0C0EB0  A4+0x72 is a long pointer; its word +0xC selects the mode
 *   0C0EB2..0C0F66  mode != 0: one probe from A4+0x8E/0x90 (+0xF4 when the
 *                   A4+0x6D byte is set), the D7-biased drop test, stand
 *   0C0F68..0C0F8E  mode == 0: state word (*(A4+0x9C)).w+8, flipped for the
 *                   A4+0x6A side, must be inside the {2,3} pair at 0x0C0E84
 *   0C0F92..0C11F2  state 2: probe pairs from A4+0xDA/0xDC then 0xDE/0xE0
 *   0C11F4..0C1454  inside but not 2: the same pairs in the other order
 *   0C1456..0C1778  outside: averages (A4+0xEA/0xE4 with 3*A4+0xE2) as the
 *                   box, probe pairs from 0xE2/0xE4, then 0xDA/0xDC, 0xDE/0xE0
 *
 * The same clamp + probe pair + drop-test body appears at 0x0C0F92 / 0x0C10C2
 * / 0x0C11F4 / 0x0C1324 / 0x0C1456 / 0x0C153C / 0x0C165A, so it is one helper
 * parameterised by the source fields and the clamp flavour.  A block that
 * gives up falls through to the next one, as the ROM's branch targets do. */
#include "ea_auto.h"

/* .w op on a data register: only the low word changes. */
static uint32_t wadd(uint32_t reg, uint32_t v)
{
    return set_word(reg, (uint16_t)(reg + v));
}

/* ext.l Dn: the low word, sign extended. */
static uint32_t sext16(uint32_t v)
{
    return (uint32_t)(int32_t)(int16_t)(uint16_t)v;
}

/* asr.w #n on the low word. */
static uint32_t asr16(uint32_t reg, unsigned n)
{
    return set_word(reg, (uint16_t)((int16_t)(uint16_t)reg >> n));
}

/* cmp2.w bounds at addr: carry (bcs) means "outside". */
static int cmp2_out_w(const f3mem_t *m, uint32_t addr, uint16_t v)
{
    return (int16_t)v < (int16_t)rd16(m, addr)
        || (int16_t)v > (int16_t)rd16(m, addr + 2u);
}

/* cmp2.l bounds at addr: carry (bcs) means "outside". */
static int cmp2_out_l(const f3mem_t *m, uint32_t addr, uint32_t v)
{
    return v < rd32(m, addr) || v > rd32(m, addr + 2u);
}

/* move.w X,d0 ; sub.w $8a(a4),d0 ; cmpi.w #$c,d0 ; bcc L (d0 = X) : d0 = $8a(a4) */
static uint16_t xclamp(const f3mem_t *m, uint32_t a4, uint32_t v)
{
    uint16_t t = (uint16_t)((uint16_t)v - rd16(m, a4 + 0x08Au));

    return t >= 0x000Cu ? (uint16_t)v : rd16(m, a4 + 0x08Au);
}

/* move.w $2(a0),d1 ; sub.w (a0),d1 - thickness of the record the probe hit. */
static uint32_t hit_d1(const f3mem_t *m, uint32_t a0)
{
    return (uint32_t)(uint16_t)(rd16(m, a0 + 2u) - rd16(m, a0));
}

/* 0x0BD9F2: ground probe.  D1.w is indexed against the A5+0x99A/0x99E grid
 * width and D0.w against A5+0x998/0x99C, then the map base long at A5+0x988
 * selects the row list.  Returns D0.w = the surface value and A0 = the map
 * record; out of the grid it is D0.w = 0x7FFF, A0 = 0x0BD5AE.  D2-D4/A1-A2
 * are saved, and the A0 object list at 0x41B81C can override both. */
static void ea_probe(const f3mem_t *m, f3regs_t *r, uint32_t ret)
{
    uint32_t a5 = r->a[5];
    uint32_t d1 = r->d[1];
    uint32_t d4 = set_word(r->d[4], (uint16_t)r->d[1]);   /* 0x0BD9FA move.w d1,d4 */
    uint32_t a0, a1, a2;
    uint16_t n;
    uint16_t cnt;

    r->a[7] -= 4u;                                        /* the jsr */
    wr32(m, r->a[7], ret);
    r->a[7] -= 20u;                                       /* 0x0BD9F2 movem.l */
    wr32(m, r->a[7] + 0u, r->d[2]);
    wr32(m, r->a[7] + 4u, r->d[3]);
    wr32(m, r->a[7] + 8u, r->d[4]);
    wr32(m, r->a[7] + 12u, r->a[1]);
    wr32(m, r->a[7] + 16u, r->a[2]);

    /* 0x0BD9FA..0x0BDA06: depth index, two rows per unit */
    d4 = wadd(d4, (uint32_t)(0u - rd16(m, a5 + 0x99Au)));  /* sub.w $99a(a5),d4 */
    d4 = asr16(d4, 1);
    a0 = 0x000BD5AEu;
    if ((uint16_t)d4 >= rd16(m, a5 + 0x99Eu)) {            /* bcc 0x0BD9BC */
        r->d[0] = set_word(r->d[0], 0x7FFFu);
    } else {
        d4 = set_word(d4, (uint16_t)r->d[0]);               /* 0x0BDA08 move.w d0,d4 */
        d4 = wadd(d4, (uint32_t)(0u - rd16(m, a5 + 0x998u))); /* sub.w $998(a5),d4 */
        d4 = asr16(d4, 2);                                  /* asr.w #2 */
        if ((uint16_t)d4 >= rd16(m, a5 + 0x99Cu)) {          /* bcc 0x0BD9BC */
            r->d[0] = set_word(r->d[0], 0x7FFFu);
        } else {
            /* 0x0BDA1A walk: the row table gives a word index, negated, and the
             * word it addresses is the offset of the row's cell list.  Both
             * indexed operands use brief extension word 0x4200/0x42FE: index
             * D4.w sign-extended with SCALE 2. */
            a0 = rd32(m, a5 + 0x988u);
            d4 = set_word(d4, rd16(m, a0 + (uint32_t)((int32_t)(int16_t)(uint16_t)d4 * 2)));
            d4 = set_word(d4, (uint16_t)(0u - (uint16_t)d4));      /* neg.w d4 */
            /* 0x0BDA20 adda.w -$2(a0,d4.w*2),a0 adds the word it reads there */
            a0 = a0 + sext16(rd16(m, a0 + (uint32_t)((int32_t)(int16_t)(uint16_t)d4 * 2) - 2u));
            a0 -= 4u;                                              /* 0x0BDA24 subq.w #4,a0 */
            do {
                a0 += 4u;                                          /* 0x0BDA26 addq.w #4,a0 */
            } while ((int16_t)(uint16_t)d1 > (int16_t)rd16(m, a0)); /* cmp.w (a0),d1 ; bgt */
            r->d[0] = set_word(r->d[0], rd16(m, a0));              /* move.w (a0)+,d0 */
            a0 += 2u;
            a0 = a0 + sext16(rd16(m, a0));                         /* adda.w (a0),a0 */
        }
    }

    /* 0x0BDA30..0x0BDAD8: the D3 = 0xFF entry, so the record list at 0x41B81C
     * is scanned with the "swap the two windows" body at 0x0BDA8E. */
    a2 = 0x0041B81Cu;
    cnt = rd16(m, 0x0041B818u);                            /* 0x0BDA8E move.w $41b818.l,d3 */
    if ((cnt & 0x8000u) == 0u) {                           /* bmi 0x0BDADA */
        for (;;) {
            a1 = rd32(m, a2);                              /* 0x0BDA96 movea.l (a2)+,a1 */
            a2 += 4u;
            if (rd32(m, a1 + 0x24u) != 0u                     /* tst.l $24(a1) */
                && !cmp2_out_w(m, a1 + 0x24u, (uint16_t)r->d[0])
                && (int16_t)(uint16_t)r->d[0] > (int16_t)rd16(m, a1 + 0x2Cu)
                && (int16_t)(uint16_t)d1 <= (int16_t)rd16(m, a1 + 0x2Cu)) {
                r->d[0] = set_word(r->d[0], rd16(m, a1 + 0x2Cu));
                a0 = a1 + 0x24u;
            }
            if (!cmp2_out_w(m, a1 + 0x34u, (uint16_t)r->d[0])
                && (int16_t)(uint16_t)r->d[0] > (int16_t)rd16(m, a1 + 0x3Cu)
                && (int16_t)(uint16_t)d1 <= (int16_t)rd16(m, a1 + 0x3Cu)) {
                r->d[0] = set_word(r->d[0], rd16(m, a1 + 0x3Cu));
                a0 = a1 + 0x34u;
            }
            n = (uint16_t)(cnt - 1u);                      /* 0x0BDAD6 dbra d3 */
            if (n == 0xFFFFu)
                break;
            cnt = n;
        }
    }

    r->a[0] = a0;
    r->d[2] = rd32(m, r->a[7] + 0u);                       /* 0x0BDADA movem.l (a7)+ */
    r->d[3] = rd32(m, r->a[7] + 4u);
    r->d[4] = rd32(m, r->a[7] + 8u);
    r->a[1] = rd32(m, r->a[7] + 12u);
    r->a[2] = rd32(m, r->a[7] + 16u);
    r->a[7] += 24u;                                        /* movem + the rts */
}

/* 0x0C0F08 / 0x0C1064 / 0x0C118E / ... : the drop test and the stand.  D5 is
 * the probe's unclamped depth, D0 the surface the last probe reported. */
static void ea_stand(const f3mem_t *m, f3regs_t *r, uint32_t a4)
{
    r->d[5] = sext16(r->d[5]);                             /* ext.l d5 */
    r->d[0] = sext16(r->d[0]);                             /* ext.l d0 */
    r->d[5] = r->d[5] - 1u;                                /* subq.l #1,d5 */
    r->d[5] = r->d[5] - r->d[0];                           /* sub.l d0,d5 */
    if ((int32_t)r->d[5] < 0)                              /* blt */
        return;
    if ((int16_t)(uint16_t)r->d[5] > 0x0C)                 /* cmpi.w #$c,d5 ; bgt */
        return;
    wr16(m, a4 + 0x0A4u, (uint16_t)(rd16(m, a4 + 0x0A4u) - (uint16_t)r->d[5]));

    /* 0x0C0F0C..0x0C0F1A: A4+0xC6 is cleared when the A4+0x4A clock went up */
    {
        uint32_t d1 = rd32(m, a4 + 0x04Au) & 0xFFFF0000u; /* move.l $4a(a4),d1 ; clr.w d1 */
        d1 += rd32(m, a4 + 0x0C6u);                       /* add.l $c6(a4),d1 */
        if ((int32_t)d1 >= 0)                             /* blt 0x0C0F1C */
            wr32(m, a4 + 0x0C6u, 0u);

        /* 0x0C0F1C: the ground records are re-derived from the hit address */
        wr32(m, a4 + 0x042u, 0u);
        wr16(m, a4 + 0x046u, 0u);
        /* 0x0C0F24 cmp2.l $c0e86(pc),a0 - the encoded form is opcode + the
         * compared-register word at 0x0C0F26 + the d16 at 0x0C0F28, and (as
         * everywhere in this ROM) the PC is the displacement word's address,
         * so the bounds pair is the aligned {0x41B920,0x41D91F} at 0x0C0E88. */
        if (!cmp2_out_l(m, 0x0C0E88u, r->a[0])) {         /* bcs 0x...56 */
            uint32_t d2;

            d1 = r->a[0] - 0x24u;                         /* move.l a0,d1 ; subi.l #$24 */
            wr16(m, a4 + 0x046u, 0u);
            d2 = d1 - 0x41B920u;
            if ((uint16_t)(d2 & 0x007Fu) != 0u) {         /* andi.w #$7f,d2 ; beq */
                d1 -= 0x10u;
                wr16(m, a4 + 0x046u, 0xFFFFu);
            }
            wr32(m, a4 + 0x042u, d1);
            r->d[2] = set_word(r->d[2], (uint16_t)d2);
        }
        r->d[1] = d1;                     /* whichever address the ROM left in D1 */
    }

    wr16(m, a4 + 0x086u, 0u);
    if (!cmp2_out_w(m, r->a[5] + 0x97Au, (uint16_t)r->d[0])) /* 0x0C0F5A bcs */
        wr16(m, a4 + 0x066u, (uint16_t)r->d[0]);
}

/* One "clamp + probe pair + stand" body.  off_x/off_y are the A4 sources of
 * D4/D5, clamp is 0 (0x0C1456), 1 (0x0C0F92 style: A4+0x86 sign, then the
 * A4+0xE6/0xEE and A4+0xE8/0xF0 limits) or 2 (0x0C153C style: the 3*A4+0xE2
 * averages against A4+0xEE/0xF0).  An abort leaves the block and the caller
 * continues with the next one, exactly like the ROM's fall-through labels. */
static void ea_block(const f3mem_t *m, f3regs_t *r, uint32_t a4, int off_y,
                     int off_x, int clamp, uint32_t ret1, uint32_t ret2)
{
    r->d[4] = set_word(r->d[4], rd16(m, a4 + (uint32_t)off_y));
    r->d[5] = set_word(r->d[5], rd16(m, a4 + (uint32_t)off_x));
    r->d[3] = set_word(r->d[3], (uint16_t)r->d[5]);

    if (clamp == 1) {
        uint16_t e2 = rd16(m, a4 + 0x0E2u);

        if ((int16_t)rd16(m, a4 + 0x086u) < 0) {          /* tst.w $86(a4) ; bpl */
            if (rd16(m, a4 + 0x06Au) == 0u) {             /* tst.w $6a(a4) ; bne */
                if ((int16_t)(uint16_t)r->d[4] < (int16_t)e2)
                    r->d[4] = set_word(r->d[4], e2);
            } else if ((int16_t)(uint16_t)r->d[4] > (int16_t)e2) {
                r->d[4] = set_word(r->d[4], e2);
            }
        }
        {
            uint16_t t = rd16(m, a4 + 0x0E6u);
            uint16_t lim = rd16(m, a4 + 0x0EEu);

            if ((int16_t)t > (int16_t)lim)
                t = lim;
            if ((int16_t)(uint16_t)r->d[4] < (int16_t)t)
                r->d[4] = set_word(r->d[4], t);
            t = rd16(m, a4 + 0x0E8u);
            lim = rd16(m, a4 + 0x0F0u);
            if ((int16_t)t < (int16_t)lim)
                t = lim;
            if ((int16_t)(uint16_t)r->d[4] > (int16_t)t)
                r->d[4] = set_word(r->d[4], t);
        }
    } else if (clamp == 2) {
        uint16_t e2 = rd16(m, a4 + 0x0E2u);
        uint16_t t;

        r->d[3] = set_word(r->d[3], (uint16_t)(rd16(m, a4 + 0x0EAu) + rd16(m, a4 + 0x0E4u)));
        r->d[3] = asr16(r->d[3], 1);
        t = (uint16_t)((uint16_t)(e2 + e2) + e2);
        t = (uint16_t)(t + rd16(m, a4 + 0x0EEu));
        t = (uint16_t)((int16_t)t >> 2);
        if ((int16_t)(uint16_t)r->d[4] < (int16_t)t)
            r->d[4] = set_word(r->d[4], t);
        t = (uint16_t)((uint16_t)(e2 + e2) + e2);
        t = (uint16_t)(t + rd16(m, a4 + 0x0F0u));
        t = (uint16_t)((int16_t)t >> 2);
        if ((int16_t)(uint16_t)r->d[4] > (int16_t)t)
            r->d[4] = set_word(r->d[4], t);
    }

    r->d[4] = wadd(r->d[4], rd16(m, a4 + 0x0A0u));
    r->d[3] = wadd(r->d[3], rd16(m, a4 + 0x0CCu));
    r->d[5] = wadd(r->d[5], rd16(m, a4 + 0x0A4u));

    /* first probe: depth from D3 */
    r->d[0] = set_word(r->d[0], xclamp(m, a4, r->d[3]));
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);
    r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
    r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + r->d[7]));
    ea_probe(m, r, ret1);
    r->d[1] = set_word(r->d[1], hit_d1(m, r->a[0]));
    if ((uint16_t)r->d[1] <= 8u)                            /* cmpi.w #$8,d1 ; bls */
        return;

    /* second probe: depth from D5 */
    r->d[6] = set_word(r->d[6], (uint16_t)r->d[0]);         /* move.w d0,d6 */
    r->d[0] = set_word(r->d[0], xclamp(m, a4, r->d[5]));
    r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);
    r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
    r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + r->d[7]));
    ea_probe(m, r, ret2);
    r->d[1] = set_word(r->d[1], hit_d1(m, r->a[0]));
    if ((uint16_t)r->d[1] <= 8u)
        return;

    /* cmp.w d6,d0 ; ble ; move.w d6,d0 - take the lower surface */
    if ((int16_t)(uint16_t)r->d[0] > (int16_t)(uint16_t)r->d[6])
        r->d[0] = set_word(r->d[0], (uint16_t)r->d[6]);
    ea_stand(m, r, a4);
}

int Ea_0C0E92(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a4 = r->a[4];

    /* 0x0C0E92 moveq #$0,d7 ; 0x0C0E94 tst.l $c6(a4) ; ble.w $c0ea4 */
    r->d[7] = 0u;
    if ((int32_t)rd32(m, a4 + 0x0C6u) > 0) {
        r->d[7] = set_word(r->d[7], rd16(m, a4 + 0x0C6u));  /* 0x0C0E9C */
        r->d[7] = set_word(r->d[7], (uint16_t)(0u - (uint16_t)r->d[7])); /* neg.w */
        r->d[7] = set_word(r->d[7], (uint16_t)(r->d[7] - 1u));          /* subq.w #1 */
    }
    r->d[7] = set_word(r->d[7], (uint16_t)(r->d[7] - 4u));  /* 0x0C0EA4 */

    /* 0x0C0EA6 tst.w ([$72,a4],$c) ; beq.w $c0f68 */
    if (rd16(m, rd32(m, a4 + 0x072u) + 0x0Cu) != 0u) {
        /* ---- on the surface path, 0x0C0EB2..0x0C0F66 ------------------- */
        r->d[4] = set_word(r->d[4], rd16(m, a4 + 0x08Eu));
        r->d[5] = set_word(r->d[5], rd16(m, a4 + 0x090u));
        r->d[4] = wadd(r->d[4], rd16(m, a4 + 0x0A0u));
        if (rd8(m, a4 + 0x06Du) != 0u)                      /* tst.b $6d(a4) */
            r->d[5] = set_word(r->d[5], rd16(m, a4 + 0x0F4u));
        r->d[5] = wadd(r->d[5], rd16(m, a4 + 0x0A4u));
        r->d[0] = set_word(r->d[0], xclamp(m, a4, r->d[5]));
        r->d[1] = set_word(r->d[1], (uint16_t)r->d[0]);
        r->d[0] = set_word(r->d[0], (uint16_t)r->d[4]);
        r->d[1] = set_word(r->d[1], (uint16_t)(r->d[1] + r->d[7]));
        ea_probe(m, r, 0x0C0EECu);
        r->d[1] = set_word(r->d[1], hit_d1(m, r->a[0]));
        if ((uint16_t)r->d[1] > 8u)                         /* cmpi.w #$8,d1 ; bls */
            ea_stand(m, r, a4);
    } else {
        /* ---- 0x0C0F68: the mode word out of the A4+0x9C record --------- */
        r->d[0] = set_word(r->d[0], rd16(m, rd32(m, a4 + 0x09Cu) + 8u));
        if ((uint16_t)r->d[0] <= 3u) {                      /* cmpi.w #$3,d0 ; bhi */
            if (rd16(m, a4 + 0x06Au) != 0u)                 /* tst.w $6a(a4) ; beq */
                r->d[0] = set_word(r->d[0], (uint16_t)((uint16_t)r->d[0] ^ 1u));
        }
        /* 0x0C0F80 cmp2.w $c0e82(pc),d0 ; bcs.w $c1456 - same rule, the d16
         * is at 0x0C0F84 so the {2,3} pair at 0x0C0E84 is the range. */
        if (cmp2_out_w(m, 0x0C0E84u, (uint16_t)r->d[0])) {
            /* 0x0C1456: state 3, A4+0xE2/0xE4 with no sign clamp */
            ea_block(m, r, a4, 0x0E2, 0x0E4, 0, 0x0C148Cu, 0x0C14B6u);
            ea_block(m, r, a4, 0x0DA, 0x0DC, 2, 0x0C15AAu, 0x0C15D4u);
            ea_block(m, r, a4, 0x0DE, 0x0E0, 2, 0x0C16C8u, 0x0C16F2u);
        } else if ((uint16_t)r->d[0] == 2u) {               /* cmpi.w #$2,d0 ; bne */
            /* 0x0C0F92: state 2, A4+0xDA/0xDC then A4+0xDE/0xE0 */
            ea_block(m, r, a4, 0x0DA, 0x0DC, 1, 0x0C100Cu, 0x0C103Cu);
            ea_block(m, r, a4, 0x0DE, 0x0E0, 1, 0x0C113Cu, 0x0C116Cu);
        } else {
            /* 0x0C11F4: states -1/0/1, A4+0xDE/0xE0 then A4+0xDA/0xDC */
            ea_block(m, r, a4, 0x0DE, 0x0E0, 1, 0x0C126Eu, 0x0C129Eu);
            ea_block(m, r, a4, 0x0DA, 0x0DC, 1, 0x0C139Eu, 0x0C13CEu);
        }
    }

    r->a[7] = sp + 4u;                                      /* rts pops the PC */
    return 0;
}
