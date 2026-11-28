/* ROM 0x0BDB38: pick the range record under a pushed position.  Out of the
 * per-strip index table at A5+0x98C it walks that strip's key table for the
 * key covering the pushed X, then scans the object list at 0x41B81C keeping
 * the best {min,max} record pointer; the results replace the two pushed
 * arguments: word (SP+4) = found key, long (SP+6) = record pointer. */
#include "ea_auto.h"

/* asr.w #n on a 16-bit operand: arithmetic, rounds towards -inf. */
static uint16_t asr_w(uint16_t v, unsigned n)
{
    int16_t s = (int16_t)v;
    return s >= 0 ? (uint16_t)(s >> n) : (uint16_t)~((uint16_t)(~s) >> n);
}

/* Word operand sign-extended to a 32-bit address delta (adda.w / index). */
static int32_t sx16(uint16_t v)
{
    return (int32_t)(int16_t)v;
}

/* 0x0BDBEE..0x0BDC36 (D3 != 0): one list entry.  +0x24 holds the range pair
 * the object owns (long non-zero gates it), +0x26 its key and +0x2C/+0x2E
 * its box; the second slot repeats that at +0x34/+0x36/+0x3C/+0x3E.  A hit
 * replaces the key in *d0 and points *a0 at the box.  The second test reads
 * the *d0 the first one may have just written, as the ROM re-loads D0. */
static void scan_entry_fwd(const f3mem_t *m, uint32_t a1, uint16_t d4,
                           uint16_t d1, uint16_t d2, uint16_t *d0,
                           uint32_t *a0)
{
    if (rd32(m, a1 + 0x24u) != 0u &&
        (int16_t)d4 >= (int16_t)rd16(m, a1 + 0x26u) &&
        (int16_t)*d0 < (int16_t)rd16(m, a1 + 0x26u) &&
        (int16_t)d1 <= (int16_t)rd16(m, a1 + 0x2Eu) &&
        (int16_t)d2 >= (int16_t)rd16(m, a1 + 0x2Cu)) {
        *d0 = rd16(m, a1 + 0x26u);                    /* 0x0BDC0E */
        *a0 = a1 + 0x2Cu;                             /* 0x0BDC12 */
    }
    if ((int16_t)d4 >= (int16_t)rd16(m, a1 + 0x36u) &&
        (int16_t)*d0 < (int16_t)rd16(m, a1 + 0x36u) &&
        (int16_t)d1 <= (int16_t)rd16(m, a1 + 0x3Eu) &&
        (int16_t)d2 >= (int16_t)rd16(m, a1 + 0x3Cu)) {
        *d0 = rd16(m, a1 + 0x36u);                    /* 0x0BDC2E */
        *a0 = a1 + 0x3Cu;                             /* 0x0BDC32 */
    }
}

/* 0x0BDC4E..0x0BDC96 (D3 == 0): the alternate entry test.  The box is
 * +0x24/+0x26 with the limit at +0x2E, and +0x34/+0x36 with +0x3E; D2 and D4
 * are exchanged against D0 and D1.  Unreachable from this entry (D3 is set to
 * -1 at 0x0BDB3C) but kept register-faithful. */
static void scan_entry_rev(const f3mem_t *m, uint32_t a1, uint16_t d4,
                           uint16_t d1, uint16_t d2, uint16_t *d0,
                           uint32_t *a0)
{
    if (rd32(m, a1 + 0x24u) != 0u &&
        (int16_t)d2 >= (int16_t)rd16(m, a1 + 0x24u) &&
        (int16_t)d1 <= (int16_t)rd16(m, a1 + 0x26u) &&
        (int16_t)*d0 < (int16_t)rd16(m, a1 + 0x2Eu) &&
        (int16_t)d4 >= (int16_t)rd16(m, a1 + 0x2Eu)) {
        *d0 = rd16(m, a1 + 0x2Eu);                    /* 0x0BDC6E */
        *a0 = a1 + 0x24u;                             /* 0x0BDC72 */
    }
    if ((int16_t)d2 >= (int16_t)rd16(m, a1 + 0x34u) &&
        (int16_t)d1 <= (int16_t)rd16(m, a1 + 0x36u) &&
        (int16_t)*d0 < (int16_t)rd16(m, a1 + 0x3Eu) &&
        (int16_t)d4 >= (int16_t)rd16(m, a1 + 0x3Eu)) {
        *d0 = rd16(m, a1 + 0x3Eu);                    /* 0x0BDC8E */
        *a0 = a1 + 0x34u;                             /* 0x0BDC92 */
    }
}

int Ea_0BDB38(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp0 = r->a[7];                  /* entry SP, W = return address */
    uint32_t sp = sp0 - 32u;                 /* 0x0BDB38 movem.l d0-d4/a0-a2 */
    uint32_t a5 = r->a[5];
    uint16_t d0, d1, d2, d4;
    uint16_t d3 = 0xFFFFu;                   /* 0x0BDB3C moveq #$ff,d3 (.w used) */
    uint32_t a0, a1, a2;

    wr32(m, sp + 0u, r->d[0]);
    wr32(m, sp + 4u, r->d[1]);
    wr32(m, sp + 8u, r->d[2]);
    wr32(m, sp + 12u, r->d[3]);
    wr32(m, sp + 16u, r->d[4]);
    wr32(m, sp + 20u, r->a[0]);
    wr32(m, sp + 24u, r->a[1]);
    wr32(m, sp + 28u, r->a[2]);

    d0 = rd16(m, sp + 0x24u);                /* 0x0BDB3E move.w $24(a7),d0 */
    d0 = (uint16_t)(d0 - rd16(m, a5 + 0x0998u));   /* 0x0BDB42 sub.w $998(a5) */
    d0 = asr_w(d0, 2);                       /* 0x0BDB46 asr.w #2 */
    if ((uint16_t)d0 >= rd16(m, a5 + 0x099Cu)) {   /* 0x0BDB48/0x0BDB4C bcc */
        d0 = 0x8000u;                        /* 0x0BDCA8 move.w #$8000,d0 */
        a0 = 0x0BD5AEu;                      /* 0x0BDCAC movea.l #$bd5ae,a0 */
    } else {
        /* 0x0BDB50..0x0BDB86: Y (arg2) then X (arg1) scaled and clamped */
        d2 = rd16(m, sp + 0x28u);            /* 0x0BDB50 move.w $28(a7),d2 */
        d2 = (uint16_t)(d2 - rd16(m, a5 + 0x099Au));  /* 0x0BDB54 */
        if (d2 & 0x8000u) {                  /* 0x0BDB58 bpl */
            d2 = 0u;                         /* 0x0BDB5A moveq #$0,d2 */
        } else {
            d2 = asr_w(d2, 1);               /* 0x0BDB5E */
            if ((uint16_t)d2 >= rd16(m, a5 + 0x099Eu)) {  /* 0x0BDB60 bcs */
                d2 = (uint16_t)(rd16(m, a5 + 0x099Eu) - 1u); /* 0x0BDB66/6A */
            }
        }
        d1 = rd16(m, sp + 0x26u);            /* 0x0BDB6C move.w $26(a7),d1 */
        d1 = (uint16_t)(d1 - rd16(m, a5 + 0x099Au));  /* 0x0BDB70 */
        if (d1 & 0x8000u) {                  /* 0x0BDB74 bpl */
            d1 = 0u;                         /* 0x0BDB76 moveq #$0,d1 */
        } else {
            d1 = asr_w(d1, 1);               /* 0x0BDB7A */
            if ((uint16_t)d1 >= rd16(m, a5 + 0x099Eu)) {  /* 0x0BDB7C bcs */
                d1 = (uint16_t)(rd16(m, a5 + 0x099Eu) - 1u); /* 0x0BDB82/86 */
            }
        }
        a1 = rd32(m, a5 + 0x098Cu);          /* 0x0BDB88 movea.l $98c(a5),a1 */
        /* 0x0BDB8C/0x0BDB90: word index table, brief indexed EA with Dn.w*2 */
        d1 = rd16(m, a1 + (uint32_t)sx16(d1) * 2u);
        d2 = rd16(m, a1 + (uint32_t)sx16(d2) * 2u);
        d2 = (uint16_t)(d2 - d1);            /* 0x0BDB94 sub.w d1,d2 */
        d4 = rd16(m, sp + 0x24u);            /* 0x0BDB96 move.w $24(a7),d4 */
        d0 = 0x8000u;                        /* 0x0BDB9A move.w #$8000,d0 */
        a0 = 0x0BD5AEu;                      /* 0x0BDB9E movea.l #$bd5ae,a0 */
        d1 = (uint16_t)(0u - d1);            /* 0x0BDBA4 neg.w d1 */
        d1 = (uint16_t)(d1 - 1u);            /* 0x0BDBA6 subq.w #1,d1 */
        d1 = (uint16_t)(d1 + d1);            /* 0x0BDBA8 add.w d1,d1 */
        /* 0x0BDBAA..0x0BDBB0 re-entered by the dbra (disp 0xFFE4 from
         * 0x0BDBC6 -> 0x0BDBAA), so A2 is RE-ANCHORED from A1 every row. */
        for (;;) {
            a2 = a1 + (uint32_t)sx16(rd16(m, a1 + (uint32_t)sx16(d1)));
            a2 -= 4u;                        /* 0x0BDBB0 subq.w #4,a2 */
            a2 += 4u;                        /* 0x0BDBB2 addq.w #4,a2 */
            /* 0x0BDBB4/0x0BDBB6: advance while the key is above D4 (signed) */
            while ((int16_t)d4 < (int16_t)rd16(m, a2)) {
                a2 += 4u;
            }
            /* 0x0BDBB8/0x0BDBBA: keep it when D0 is below the key (signed) */
            if (!((int16_t)d0 >= (int16_t)rd16(m, a2))) {
                d0 = rd16(m, a2);            /* 0x0BDBBC move.w (a2)+,d0 */
                a2 += 2u;
                /* 0x0BDBBE/0x0BDBC0: lea (a2),a0 ; adda.w (a2),a0 */
                a0 = a2 + (uint32_t)sx16(rd16(m, a2));
            }
            d1 = (uint16_t)(d1 - 2u);        /* 0x0BDBC2 subq.w #2,d1 */
            d2 = (uint16_t)(d2 - 1u);        /* 0x0BDBC4 dbra d2 */
            if (d2 == 0xFFFFu) {
                break;
            }
        }
    }

    /* 0x0BDBC8 tst.w $41B818.l ; 0x0BDBCE bmi (bit 15 set -> no scan) */
    if ((rd16(m, 0x41B818u) & 0x8000u) == 0u) {
        a2 = 0x41B81Cu;                      /* 0x0BDBD2 lea $41b81c.l,a2 */
        d1 = rd16(m, sp + 0x26u);            /* 0x0BDBD8 move.w $26(a7),d1 */
        d2 = rd16(m, sp + 0x28u);            /* 0x0BDBDC move.w $28(a7),d2 */
        d4 = rd16(m, sp + 0x24u);            /* 0x0BDBE0 move.w $24(a7),d4 */
        if (d3 == 0u) {                      /* 0x0BDBE4 tst.w d3 / beq */
            d3 = rd16(m, 0x41B818u);         /* 0x0BDC48 */
            for (;;) {
                a1 = rd32(m, a2);            /* 0x0BDC4E movea.l (a2)+,a1 */
                a2 += 4u;
                scan_entry_rev(m, a1, d4, d1, d2, &d0, &a0);
                d3 = (uint16_t)(d3 - 1u);    /* 0x0BDC96 dbra d3 */
                if (d3 == 0xFFFFu) {
                    break;
                }
            }
        } else {
            d3 = rd16(m, 0x41B818u);         /* 0x0BDBE8 move.w $41b818.l,d3 */
            for (;;) {
                a1 = rd32(m, a2);            /* 0x0BDBEE movea.l (a2)+,a1 */
                a2 += 4u;
                scan_entry_fwd(m, a1, d4, d1, d2, &d0, &a0);
                d3 = (uint16_t)(d3 - 1u);    /* 0x0BDC36 dbra d3 */
                if (d3 == 0xFFFFu) {
                    break;
                }
            }
        }
    }

    wr16(m, sp + 0x24u, d0);                 /* 0x0BDC3A move.w d0,$24(a7) */
    wr32(m, sp + 0x26u, a0);                 /* 0x0BDC3E move.l a0,$26(a7) */

    r->d[0] = rd32(m, sp + 0u);              /* 0x0BDC42 movem.l (a7)+,d0-d4/a0-a2 */
    r->d[1] = rd32(m, sp + 4u);
    r->d[2] = rd32(m, sp + 8u);
    r->d[3] = rd32(m, sp + 12u);
    r->d[4] = rd32(m, sp + 16u);
    r->a[0] = rd32(m, sp + 20u);
    r->a[1] = rd32(m, sp + 24u);
    r->a[2] = rd32(m, sp + 28u);
    r->a[7] = sp0 + 4u;                      /* the rts pops the return address */
    return 0;
}
