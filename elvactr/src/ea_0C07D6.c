/* ROM 0x0C07D6: push one record's A4+0xA0 coordinate out of the platform edge
 * it overlaps (four mirrored tile-map + object-list probes: 0x0BD8A4 /
 * 0x0BD9C8 on the A5+0x98C / A5+0x990 maps and 0x0BDB38 / 0x0BDD0E on the
 * maps plus the 0x41B81C object list), flipping and quartering the A4+0xC2
 * step, then re-arms the A4+0xCE sentinel with 0x80007FFF. */
#include "ea_auto.h"

/* ---- 68k idioms ------------------------------------------------------- */

static inline int16_t sig(uint32_t v) { return (int16_t)(uint16_t)v; }

static inline uint16_t w16(const f3mem_t *m, uint32_t base, uint32_t off)
{
    return rd16(m, base + off);
}

/* CMP2.w: the branch's BCS means "outside" the SIGNED, inclusive bounds
 * {lo.w, hi.w} read at the operand (same rule as src/enemy/obj_119BBE.c). */
static int cmp2_inside(uint16_t v, uint16_t lo, uint16_t hi)
{
    return !((int16_t)v < (int16_t)lo || (int16_t)v > (int16_t)hi);
}

/* 0C08A4..0C08C2 (and the nine copies): move.b $6c(a4),d0 ; st.b $6c(a4) ;
 * then, when the A4+0xC2 step points the other way (neg_on_positive picks
 * "ble" or "bpl"), negate it and quarter it if the A4+0x6C flag was clear. */
static uint32_t reflect_step(const f3mem_t *m, uint32_t a4, uint32_t d0,
                             int neg_on_positive)
{
    int32_t c2;

    d0 = (d0 & 0xFFFFFF00u) | rd8(m, a4 + 0x6Cu);   /* move.b $6c(a4),d0 */
    wr8(m, a4 + 0x6Cu, 0xFFu);                      /* st.b   $6c(a4) */
    c2 = (int32_t)rd32(m, a4 + 0xC2u);              /* tst.l  $c2(a4) */
    if (neg_on_positive ? (c2 <= 0) : (c2 >= 0))
        return d0;
    wr32(m, a4 + 0xC2u, (uint32_t)-c2);             /* neg.l  $c2(a4) */
    if ((d0 & 0xFFu) != 0)                          /* tst.b d0 ; bne */
        return d0;
    d0 = (uint32_t)((int32_t)(uint32_t)-c2 >> 2);   /* move.l $c2(a4),d0 ; asr.l #2 */
    wr32(m, a4 + 0xC2u, d0);
    return d0;
}

/* ---- callee 0x0BD8A4 (entry with D3 = 0) ------------------------------ */
/* Tile map A5+0x98C probe, then the object list at 0x41B81C; D0.w = hit
 * value (0x8000 when out of range) and A0 = hit record (0xBD5AE default). */
static uint16_t h_bd8a4(const f3mem_t *m, uint32_t a5, uint16_t d0, uint16_t d1,
                        uint32_t *a0_out)
{
    uint16_t d2 = d0;                                   /* 0BD8AA */
    uint16_t d3, d4;
    uint32_t a0, a1, a2;

    d4 = (uint16_t)(d1 - w16(m, a5, 0x998u));           /* 0BD8AC */
    d4 = (uint16_t)(sig(d4) >> 2);                      /* 0BD8B2 */
    if (d4 >= w16(m, a5, 0x99Cu))                       /* 0BD8B4 bcc */
        goto oob;
    d4 = (uint16_t)(d0 - w16(m, a5, 0x99Au));           /* 0BD8BA */
    d4 = (uint16_t)(sig(d4) >> 1);                      /* 0BD8C0 */
    if (d4 >= w16(m, a5, 0x99Eu))                       /* 0BD8C2 bcc */
        goto oob;

    a0 = rd32(m, a5 + 0x98Cu);                          /* 0BD8C8 */
    d4 = rd16(m, a0 + (uint32_t)(int32_t)sig(d4) * 2u); /* 0BD8F6 (a0,d4.w*2) */
    d4 = (uint16_t)-(int16_t)d4;                        /* 0BD8FA neg.w */
    a0 += (uint32_t)(int32_t)sig(d4) * 2u - 2u;         /* 0BD8FC adda.w -2(a0,d4.w*2) */
    a0 -= 4u;                                           /* 0BD900 subq.w #4,a0 */
    goto scan;

oob:
    d0 = 0x8000u;                                       /* 0BD896 */
    a0 = 0xBD5AEu;                                      /* 0BD89A */
    goto objects;                                       /* 0BD8A0 bra.w $bd90c */
scan:
    a0 += 4u;                                           /* 0BD902 */
    while (sig(d1) < sig(rd16(m, a0)))                   /* 0BD904 blt */
        a0 += 4u;
    d0 = rd16(m, a0);                                    /* 0BD908 move.w (a0)+,d0 */
    a0 += 2u;
    a0 += (uint32_t)(int32_t)sig(rd16(m, a0));            /* 0BD90A adda.w (a0),a0 */

objects:
    a2 = 0x41B81Cu;                                      /* 0BD90C */
    if (sig(rd16(m, 0x41B818u)) >= 0) {                  /* 0BD916/0BD91C bmi */
        d3 = rd16(m, 0x41B818u);
        for (;;) {
            a1 = rd32(m, a2);                            /* 0BD920 */
            a2 += 4u;
            if (rd32(m, a1 + 0x24u) != 0                     /* 0BD922 beq */
                && cmp2_inside(d2, w16(m, a1, 0x2Cu), w16(m, a1, 0x2Eu))
                && sig(d0) < sig(w16(m, a1, 0x26u))          /* 0BD930 bge */
                && sig(d1) >= sig(w16(m, a1, 0x26u))) {      /* 0BD936 blt */
                d0 = w16(m, a1, 0x26u);                      /* 0BD93C */
                a0 = a1 + 0x2Cu;                             /* 0BD940 */
            }
            if (cmp2_inside(d2, w16(m, a1, 0x3Cu), w16(m, a1, 0x3Eu))  /* 0BD944 */
                && sig(d0) < sig(w16(m, a1, 0x36u))                /* 0BD94C */
                && sig(d1) >= sig(w16(m, a1, 0x36u))) {            /* 0BD952 */
                d0 = w16(m, a1, 0x36u);                            /* 0BD958 */
                a0 = a1 + 0x3Cu;                                   /* 0BD95C */
            }
            if ((uint16_t)--d3 == 0xFFFFu)                         /* 0BD960 dbra */
                break;
        }
    }
    *a0_out = a0;                                        /* 0BD964 movem + rts */
    return d0;
}

/* ---- callee 0x0BD9C8 (entry with D3 = 0) ------------------------------ */
/* Same walk on the A5+0x990 map, mirrored comparisons; 0x7FFF default. */
static uint16_t h_bd9c8(const f3mem_t *m, uint32_t a5, uint16_t d0, uint16_t d1,
                        uint32_t *a0_out)
{
    uint16_t d2 = d0;                                   /* 0BD9CE */
    uint16_t d3, d4;
    uint32_t a0, a1, a2;

    d4 = (uint16_t)(d1 - w16(m, a5, 0x998u));           /* 0BD9D0 */
    d4 = (uint16_t)(sig(d4) >> 2);                      /* 0BD9D6 */
    if (d4 >= w16(m, a5, 0x99Cu))                       /* 0BD9D8 bcc */
        goto oob;
    d4 = (uint16_t)(d0 - w16(m, a5, 0x99Au));           /* 0BD9DE */
    d4 = (uint16_t)(sig(d4) >> 1);                      /* 0BD9E4 */
    if (d4 >= w16(m, a5, 0x99Eu))                       /* 0BD9E6 bcc */
        goto oob;

    a0 = rd32(m, a5 + 0x990u);                          /* 0BD9EC */
    d4 = rd16(m, a0 + (uint32_t)(int32_t)sig(d4) * 2u); /* 0BDA1A */
    d4 = (uint16_t)-(int16_t)d4;                        /* 0BDA1E neg.w */
    a0 += (uint32_t)(int32_t)sig(d4) * 2u - 2u;         /* 0BDA20 adda.w -2(a0,d4.w*2) */
    a0 -= 4u;                                           /* 0BDA24 */
    goto scan;

oob:
    d0 = 0x7FFFu;                                       /* 0BD9BC */
    a0 = 0xBD5AEu;                                      /* 0BD9C0 */
    goto objects;                                       /* 0BD9C6 bra.b $bda30 */
scan:
    a0 += 4u;                                           /* 0BDA26 */
    while (sig(d1) > sig(rd16(m, a0)))                   /* 0BDA28 bgt */
        a0 += 4u;
    d0 = rd16(m, a0);                                    /* 0BDA2C move.w (a0)+,d0 */
    a0 += 2u;
    a0 += (uint32_t)(int32_t)sig(rd16(m, a0));            /* 0BDA2E adda.w (a0),a0 */

objects:
    a2 = 0x41B81Cu;                                      /* 0BDA30 */
    if (sig(rd16(m, 0x41B818u)) >= 0) {                  /* 0BDA3A/0BDA40 bmi */
        d3 = rd16(m, 0x41B818u);
        for (;;) {
            a1 = rd32(m, a2);                            /* 0BDA44 */
            a2 += 4u;
            if (rd32(m, a1 + 0x24u) != 0                     /* 0BDA46 beq */
                && cmp2_inside(d2, w16(m, a1, 0x2Cu), w16(m, a1, 0x2Eu))
                && sig(d0) > sig(w16(m, a1, 0x24u))          /* 0BDA54 ble */
                && sig(d1) <= sig(w16(m, a1, 0x24u))) {      /* 0BDA5A bgt */
                d0 = w16(m, a1, 0x24u);                      /* 0BDA60 */
                a0 = a1 + 0x2Cu;                             /* 0BDA64 */
            }
            if (cmp2_inside(d2, w16(m, a1, 0x3Cu), w16(m, a1, 0x3Eu))  /* 0BDA68 */
                && sig(d0) > sig(w16(m, a1, 0x34u))                /* 0BDA70 */
                && sig(d1) <= sig(w16(m, a1, 0x34u))) {            /* 0BDA76 */
                d0 = w16(m, a1, 0x34u);                            /* 0BDA7C */
                a0 = a1 + 0x3Cu;                                   /* 0BDA80 */
            }
            if ((uint16_t)--d3 == 0xFFFFu)                         /* 0BDA84 dbra */
                break;
        }
    }
    *a0_out = a0;                                        /* 0BDA88 movem + rts */
    return d0;
}

/* ---- callee 0x0BDB38 (always entered with D3 = 0xFF) ------------------ */
/* Stack args (in push order) at $24/$26/$28(a7): column, row, other row.
 * Writes D0.w and A0 back into those two argument slots. */
static void h_bdb38(const f3mem_t *m, uint32_t a5, uint16_t arg0, uint16_t arg1,
                    uint16_t arg2, uint16_t *d0_out, uint32_t *a0_out)
{
    uint16_t d0, d1, d2, d3, d4;
    uint32_t a0, a1, a2;

    d3 = 0x00FFu;                                       /* 0BDB3C */
    d0 = (uint16_t)(arg0 - w16(m, a5, 0x998u));         /* 0BDB3E */
    d0 = (uint16_t)(sig(d0) >> 2);                      /* 0BDB46 */
    if (d0 >= w16(m, a5, 0x99Cu)) {                     /* 0BDB4C bcc */
        d0 = 0x8000u;                                   /* 0BDCA8 */
        a0 = 0xBD5AEu;                                  /* 0BDCAC */
        goto objects;
    }
    d2 = (uint16_t)(arg2 - w16(m, a5, 0x99Au));         /* 0BDB50 */
    if (sig(d2) < 0)                                    /* 0BDB58 bpl */
        d2 = 0;
    else {
        d2 = (uint16_t)(sig(d2) >> 1);                  /* 0BDB5E */
        if (d2 >= w16(m, a5, 0x99Eu))                   /* 0BDB64 bcs */
            d2 = (uint16_t)(w16(m, a5, 0x99Eu) - 1u);   /* 0BDB66 */
    }
    d1 = (uint16_t)(arg1 - w16(m, a5, 0x99Au));         /* 0BDB6C */
    if (sig(d1) < 0)                                    /* 0BDB74 bpl */
        d1 = 0;
    else {
        d1 = (uint16_t)(sig(d1) >> 1);                  /* 0BDB7A */
        if (d1 >= w16(m, a5, 0x99Eu))                   /* 0BDB80 bcs */
            d1 = (uint16_t)(w16(m, a5, 0x99Eu) - 1u);   /* 0BDB82 */
    }
    a1 = rd32(m, a5 + 0x98Cu);                          /* 0BDB88 */
    d1 = rd16(m, a1 + (uint32_t)(int32_t)sig(d1) * 2u); /* 0BDB8C */
    d2 = rd16(m, a1 + (uint32_t)(int32_t)sig(d2) * 2u); /* 0BDB90 */
    d2 = (uint16_t)(d2 - d1);                           /* 0BDB94 */
    d4 = arg0;                                          /* 0BDB96 */
    d0 = 0x8000u;                                       /* 0BDB9A */
    a0 = 0xBD5AEu;                                      /* 0BDB9E */
    d1 = (uint16_t)-(int16_t)d1;                        /* 0BDBA4 neg.w */
    d1 = (uint16_t)(d1 - 1u);                           /* 0BDBA6 */
    d1 = (uint16_t)(d1 + d1);                           /* 0BDBA8 */
    for (;;) {                                          /* 0BDBAA */
        a2 = a1 + (uint32_t)(int32_t)sig(d1);           /* adda.w (a1,d1.w) */
        a2 -= 4u;                                       /* 0BDBB0 */
        do {                                            /* 0BDBB2 */
            a2 += 4u;
        } while (sig(d4) < sig(rd16(m, a2)));            /* 0BDBB4 blt */
        if (sig(d0) < sig(rd16(m, a2))) {                /* 0BDBB8 bge */
            d0 = rd16(m, a2);                            /* 0BDBBC */
            a2 += 2u;
            a0 = a2;                                     /* 0BDBBE lea (a2),a0 */
            a0 += (uint32_t)(int32_t)sig(rd16(m, a2));   /* 0BDBC0 adda.w (a2),a0 */
        }
        d1 = (uint16_t)(d1 - 2u);                        /* 0BDBC2 */
        if ((uint16_t)--d2 == 0xFFFFu)                   /* 0BDBC4 dbra */
            break;
    }

objects:
    if (sig(rd16(m, 0x41B818u)) < 0)                    /* 0BDBC8 bmi */
        goto done;
    a2 = 0x41B81Cu;                                     /* 0BDBD2 */
    d1 = arg1;                                          /* 0BDBD8 */
    d2 = arg2;                                          /* 0BDBDC */
    d4 = arg0;                                          /* 0BDBE0 */
    d3 = rd16(m, 0x41B818u);                            /* 0BDBE8 (D3 = 0xFF) */
    for (;;) {                                          /* 0BDBEE */
        a1 = rd32(m, a2);
        a2 += 4u;
        if (rd32(m, a1 + 0x24u) != 0                       /* 0BDBF0 beq */
            && sig(d4) >= sig(w16(m, a1, 0x26u))           /* 0BDBF6 blt */
            && sig(d0) < sig(w16(m, a1, 0x26u))            /* 0BDBFC bge */
            && sig(d1) <= sig(w16(m, a1, 0x2Eu))           /* 0BDC02 bgt */
            && sig(d2) >= sig(w16(m, a1, 0x2Cu))) {        /* 0BDC08 blt */
            d0 = w16(m, a1, 0x26u);                        /* 0BDC0E */
            a0 = a1 + 0x2Cu;                               /* 0BDC12 */
        }
        if (sig(d4) >= sig(w16(m, a1, 0x36u))              /* 0BDC16 blt */
            && sig(d0) < sig(w16(m, a1, 0x36u))            /* 0BDC1C bge */
            && sig(d1) <= sig(w16(m, a1, 0x3Eu))           /* 0BDC22 bgt */
            && sig(d2) >= sig(w16(m, a1, 0x3Cu))) {        /* 0BDC28 blt */
            d0 = w16(m, a1, 0x36u);                        /* 0BDC2E */
            a0 = a1 + 0x3Cu;                               /* 0BDC32 */
        }
        if ((uint16_t)--d3 == 0xFFFFu)                     /* 0BDC36 dbra */
            break;
    }
done:
    *d0_out = d0;                                       /* 0BDC3A/0BDC9A */
    *a0_out = a0;                                       /* 0BDC3E/0BDC9E */
}

/* ---- callee 0x0BDD0E (always entered with D3 = 0xFF) ------------------ */
/* Same interface as 0x0BDB38 on the A5+0x990 map, mirrored compares;
 * the tile scan starts from 0x7FFF. */
static void h_bdd0e(const f3mem_t *m, uint32_t a5, uint16_t arg0, uint16_t arg1,
                    uint16_t arg2, uint16_t *d0_out, uint32_t *a0_out)
{
    uint16_t d0, d1, d2, d3, d4;
    uint32_t a0, a1, a2;

    d3 = 0x00FFu;                                       /* 0BDD12 */
    d0 = (uint16_t)(arg0 - w16(m, a5, 0x998u));         /* 0BDD14 */
    d0 = (uint16_t)(sig(d0) >> 2);                      /* 0BDD1C */
    if (d0 >= w16(m, a5, 0x99Cu)) {                     /* 0BDD1E bcc */
        d0 = 0x7FFFu;                                   /* 0BDE7E */
        a0 = 0xBD5AEu;                                  /* 0BDE82 */
        goto objects;
    }
    d2 = (uint16_t)(arg2 - w16(m, a5, 0x99Au));         /* 0BDD26 */
    if (sig(d2) < 0)                                    /* 0BDD2E bpl */
        d2 = 0;
    else {
        d2 = (uint16_t)(sig(d2) >> 1);                  /* 0BDD34 */
        if (d2 >= w16(m, a5, 0x99Eu))                   /* 0BDD3A bcs */
            d2 = (uint16_t)(w16(m, a5, 0x99Eu) - 1u);   /* 0BDD3C */
    }
    d1 = (uint16_t)(arg1 - w16(m, a5, 0x99Au));         /* 0BDD42 */
    if (sig(d1) < 0)                                    /* 0BDD4A bpl */
        d1 = 0;
    else {
        d1 = (uint16_t)(sig(d1) >> 1);                  /* 0BDD50 */
        if (d1 >= w16(m, a5, 0x99Eu))                   /* 0BDD56 bcs */
            d1 = (uint16_t)(w16(m, a5, 0x99Eu) - 1u);   /* 0BDD58 */
    }
    a1 = rd32(m, a5 + 0x990u);                          /* 0BDD5E */
    d1 = rd16(m, a1 + (uint32_t)(int32_t)sig(d1) * 2u); /* 0BDD62 */
    d2 = rd16(m, a1 + (uint32_t)(int32_t)sig(d2) * 2u); /* 0BDD66 */
    d2 = (uint16_t)(d2 - d1);                           /* 0BDD6A */
    d4 = arg0;                                          /* 0BDD6C */
    d0 = 0x7FFFu;                                       /* 0BDD70 */
    a0 = 0xBD5AEu;                                      /* 0BDD74 */
    d1 = (uint16_t)-(int16_t)d1;                        /* 0BDD7A neg.w */
    d1 = (uint16_t)(d1 - 1u);                           /* 0BDD7C */
    d1 = (uint16_t)(d1 + d1);                           /* 0BDD7E */
    for (;;) {                                          /* 0BDD80 */
        a2 = a1 + (uint32_t)(int32_t)sig(d1);           /* adda.w (a1,d1.w) */
        a2 -= 4u;                                       /* 0BDD86 */
        do {                                            /* 0BDD88 */
            a2 += 4u;
        } while (sig(d4) > sig(rd16(m, a2)));            /* 0BDD8A bgt */
        if (sig(d0) > sig(rd16(m, a2))) {                /* 0BDD8E ble */
            d0 = rd16(m, a2);                            /* 0BDD92 */
            a2 += 2u;
            a0 = a2;                                     /* 0BDD94 lea (a2),a0 */
            a0 += (uint32_t)(int32_t)sig(rd16(m, a2));   /* 0BDD96 adda.w (a2),a0 */
        }
        d1 = (uint16_t)(d1 - 2u);                        /* 0BDD98 */
        if ((uint16_t)--d2 == 0xFFFFu)                   /* 0BDD9A dbra */
            break;
    }

objects:
    if (sig(rd16(m, 0x41B818u)) < 0)                    /* 0BDD9E bmi */
        goto done;
    a2 = 0x41B81Cu;                                     /* 0BDDA8 */
    d1 = arg1;                                          /* 0BDDAE */
    d2 = arg2;                                          /* 0BDDB2 */
    d4 = arg0;                                          /* 0BDDB6 */
    d3 = rd16(m, 0x41B818u);                            /* 0BDDBE (D3 = 0xFF) */
    for (;;) {                                          /* 0BDDC4 */
        a1 = rd32(m, a2);
        a2 += 4u;
        if (rd32(m, a1 + 0x24u) != 0                       /* 0BDDC6 beq */
            && sig(d4) <= sig(w16(m, a1, 0x24u))           /* 0BDDCC bgt */
            && sig(d0) > sig(w16(m, a1, 0x24u))            /* 0BDDD2 ble */
            && sig(d1) <= sig(w16(m, a1, 0x2Eu))           /* 0BDDD8 bgt */
            && sig(d2) >= sig(w16(m, a1, 0x2Cu))) {        /* 0BDDDE blt */
            d0 = w16(m, a1, 0x24u);                        /* 0BDDE4 */
            a0 = a1 + 0x2Cu;                               /* 0BDDE8 */
        }
        if (sig(d4) <= sig(w16(m, a1, 0x34u))              /* 0BDDEC bgt */
            && sig(d0) > sig(w16(m, a1, 0x34u))            /* 0BDDF2 ble */
            && sig(d1) <= sig(w16(m, a1, 0x3Eu))           /* 0BDDF8 bgt */
            && sig(d2) >= sig(w16(m, a1, 0x3Cu))) {        /* 0BDDFE blt */
            d0 = w16(m, a1, 0x34u);                        /* 0BDE04 */
            a0 = a1 + 0x3Cu;                               /* 0BDE08 */
        }
        if ((uint16_t)--d3 == 0xFFFFu)                     /* 0BDE0C dbra */
            break;
    }
done:
    *d0_out = d0;                                       /* 0BDE10/0BDE70 */
    *a0_out = a0;                                       /* 0BDE14/0BDE74 */
}

/* ---- the routine ------------------------------------------------------ */

int Ea_0C07D6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4], a5 = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2];
    uint32_t d3 = r->d[3], d4 = r->d[4], d7 = r->d[7];
    uint32_t a0 = r->a[0];
    uint16_t wv;

    /* 0C07D6: one-shot setup while the A4+0xCE sentinel is still armed */
    if (rd32(m, a4 + 0xCEu) == 0x80007FFFu) {
        d0 = set_word(d0, h_bd8a4(m, a5, w16(m, a4, 0xBEu), w16(m, a4, 0xBAu),
                                  &a0));
        wr16(m, a4 + 0xCEu, (uint16_t)d0);                     /* 0C07EE */
        d0 = set_word(d0, w16(m, a4, 0xBEu));                  /* 0C07F2 */
        d1 = set_word(d1, w16(m, a4, 0xBAu));                  /* 0C07F6 */
        d0 = set_word(d0, h_bd9c8(m, a5, (uint16_t)d0, (uint16_t)d1, &a0));
        wr16(m, a4 + 0xD0u, (uint16_t)d0);                     /* 0C0800 */
    }

    d3 = rd32(m, a4 + 0xC2u);                                  /* 0C0804 */
    d7 = set_word(d7, w16(m, a4, 0x6Au));                      /* 0C0808 */
    d0 = set_word(d0, w16(m, a4, 0x7Cu));                      /* 0C080C */
    d7 = set_word(d7, (uint16_t)((d7 ^ d0) & 1u));             /* 0C0810/0C0812 */
    if ((d7 & 1u) != 0)                                        /* 0C0816 bne */
        goto gate_0B4C;

    /* ---- first half: record at/above the A4+0xF0 edge ---------------- */
    d0 = set_word(d0, w16(m, a4, 0xE8u));                      /* 0C081A */
    if (sig(d0) >= sig(w16(m, a4, 0xF0u)))                     /* 0C081E bge */
        goto blk_08C8;

    /* 0C0826 */
    d0 = set_word(d0, w16(m, a4, 0xF6u));
    if ((int32_t)d3 > 0) {                                     /* 0C082A ble */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));  /* swap/sub/swap */
        d0 = set_word(d0, (uint16_t)(d0 - 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xFAu));                      /* 0C0836 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xFCu) - 4u));     /* 0C083A */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);     /* 0C0840 bge */
    d4 = set_word(d4, (uint16_t)(d1 + d2));                     /* 0C0846 */
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));                /* 0C084A */
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));      /* 0C084C */
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));      /* 0C0850 */
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));      /* 0C0854 */
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));      /* 0C0858 */
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C085C */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));                     /* 0C085E */
    d2 = set_word(d2, (uint16_t)(d2 - 2u));                     /* 0C0860 */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);     /* 0C0862 bge */
    h_bdd0e(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C086E */
    d0 = set_word(d0, wv);                                      /* 0C0874 */
    if (sig(d0) > sig(w16(m, a4, 0xD0u)))                       /* 0C0878 ble */
        d0 = set_word(d0, w16(m, a4, 0xD0u));
    d1 = set_word(d1, (uint16_t)(w16(m, a5, 0x1BC8u) + 0x140u)); /* 0C0882 */
    if (sig(d0) > sig(d1)) d0 = set_word(d0, (uint16_t)d1);      /* 0C088A ble */
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xF8u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0898 */
    if (sig(d0) < sig(d1)) {                                    /* 0C089A bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 1);
    }
    /* 0C08C4 bra.w $c0966: fall through to the 0x0C0966 block */

blk_08C8:
    /* 0C08C8 */
    d0 = set_word(d0, w16(m, a4, 0xE6u));
    if ((int32_t)d3 > 0) {                                     /* 0C08CC ble */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 - 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xEAu));                      /* 0C08D8 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xECu) - 4u));     /* 0C08DC */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdd0e(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0910 */
    d0 = set_word(d0, wv);
    if (sig(d0) > sig(w16(m, a4, 0xD0u)))                       /* 0C091A ble */
        d0 = set_word(d0, w16(m, a4, 0xD0u));
    d1 = set_word(d1, (uint16_t)(w16(m, a5, 0x1BC8u) + 0x140u)); /* 0C0924 */
    if (sig(d0) > sig(d1)) d0 = set_word(d0, (uint16_t)d1);
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xE8u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C093A */
    if (sig(d0) < sig(d1)) {                                    /* 0C093C bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 1);
    }

    /* 0C0966 */
    d0 = set_word(d0, w16(m, a4, 0xEEu));
    if ((int32_t)d3 > 0) {                                     /* 0C096A ble */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 - 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xF2u));                      /* 0C0976 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xF4u) - 4u));     /* 0C097A */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdd0e(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C09AE */
    d0 = set_word(d0, wv);
    if (sig(d0) > sig(w16(m, a4, 0xD0u)))                       /* 0C09B8 ble */
        d0 = set_word(d0, w16(m, a4, 0xD0u));
    d1 = set_word(d1, (uint16_t)(w16(m, a5, 0x1BC8u) + 0x140u)); /* 0C09C2 */
    if (sig(d0) > sig(d1)) d0 = set_word(d0, (uint16_t)d1);
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xF0u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C09D8 */
    if (sig(d0) < sig(d1)) {                                    /* 0C09DA bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 1);
    }

    /* 0C0A04 */
    d0 = set_word(d0, (uint16_t)(w16(m, a4, 0xF8u) + w16(m, a4, 0xF6u)));
    d0 = set_word(d0, (uint16_t)(sig(d0) >> 1));                /* 0C0A0C */
    if ((int32_t)d3 < 0) {                                      /* 0C0A0E bpl */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 + 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xFAu));                      /* 0C0A1A */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xFCu) - 4u));     /* 0C0A1E */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0A40 */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdb38(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0A52 */
    d0 = set_word(d0, wv);
    if (sig(d0) < sig(w16(m, a4, 0xCEu)))                       /* 0C0A5C bge */
        d0 = set_word(d0, w16(m, a4, 0xCEu));
    if (sig(d0) < sig(w16(m, a5, 0x1BC8u))) {                   /* 0C0A66 bge */
        d0 = set_word(d0, w16(m, a5, 0x1BC8u));
        d0 = set_word(d0, (uint16_t)(d0 - 1u));                 /* 0C0A70 */
    }
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xF6u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0A7A */
    if (sig(d1) < sig(d0)) {                                    /* 0C0A7C bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 0);
    }

    /* 0C0AA6 */
    d0 = set_word(d0, (uint16_t)(w16(m, a4, 0xF0u) + w16(m, a4, 0xEEu)));
    d0 = set_word(d0, (uint16_t)(sig(d0) >> 1));                /* 0C0AAE */
    if ((int32_t)d3 < 0) {                                      /* 0C0AB0 bpl */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 + 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xF2u));                      /* 0C0ABC */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xF4u) - 4u));     /* 0C0AC0 */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0AE2 */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdb38(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0AF4 */
    d0 = set_word(d0, wv);
    if (sig(d0) < sig(w16(m, a4, 0xCEu)))                       /* 0C0AFE bge */
        d0 = set_word(d0, w16(m, a4, 0xCEu));
    if (sig(d0) < sig(w16(m, a5, 0x1BC8u))) {                   /* 0C0B08 bge */
        d0 = set_word(d0, w16(m, a5, 0x1BC8u));
        d0 = set_word(d0, (uint16_t)(d0 - 1u));                 /* 0C0B12 */
    }
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xEEu) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0B1C */
    if (sig(d1) < sig(d0)) {                                    /* 0C0B1E bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 0);
    }
    goto tail;                                                  /* 0C0B48 */

    /* ---- second half: record inside the A4+0xEE edge ----------------- */
gate_0B4C:
    /* 0C0B4C */
    d0 = set_word(d0, w16(m, a4, 0xE6u));
    if (sig(d0) <= sig(w16(m, a4, 0xEEu)))                     /* 0C0B50 ble */
        goto blk_0BF8;

    /* 0C0B58 */
    d0 = set_word(d0, w16(m, a4, 0xF8u));
    if ((int32_t)d3 < 0) {                                      /* 0C0B5C bpl */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 + 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xFAu));                      /* 0C0B68 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xFCu) - 4u));     /* 0C0B6C */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0B8E */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdb38(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0BA0 */
    d0 = set_word(d0, wv);
    if (sig(d0) < sig(w16(m, a4, 0xCEu)))                       /* 0C0BAA bge */
        d0 = set_word(d0, w16(m, a4, 0xCEu));
    if (sig(d0) < sig(w16(m, a5, 0x1BC8u))) {                   /* 0C0BB4 bge */
        d0 = set_word(d0, w16(m, a5, 0x1BC8u));
        d0 = set_word(d0, (uint16_t)(d0 - 1u));                 /* 0C0BBE */
    }
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xF6u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0BC8 */
    if (sig(d1) < sig(d0)) {                                    /* 0C0BCA bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 0);
    }
    goto blk_0C94;                                              /* 0C0BF4 */

blk_0BF8:
    /* 0C0BF8 */
    d0 = set_word(d0, w16(m, a4, 0xE8u));
    if ((int32_t)d3 < 0) {                                      /* 0C0BFC bpl */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 + 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xEAu));                      /* 0C0C08 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xECu) - 4u));     /* 0C0C0C */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0C2E */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdb38(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0C40 */
    d0 = set_word(d0, wv);
    if (sig(d0) < sig(w16(m, a4, 0xCEu)))                       /* 0C0C4A bge */
        d0 = set_word(d0, w16(m, a4, 0xCEu));
    if (sig(d0) < sig(w16(m, a5, 0x1BC8u))) {                   /* 0C0C54 bge */
        d0 = set_word(d0, w16(m, a5, 0x1BC8u));
        d0 = set_word(d0, (uint16_t)(d0 - 1u));                 /* 0C0C5E */
    }
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xE6u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0C68 */
    if (sig(d1) < sig(d0)) {                                    /* 0C0C6A bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 0);
    }

blk_0C94:
    /* 0C0C94 */
    d0 = set_word(d0, w16(m, a4, 0xF0u));
    if ((int32_t)d3 < 0) {                                      /* 0C0C98 bpl */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 + 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xF2u));                      /* 0C0CA4 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xF4u) - 4u));     /* 0C0CA8 */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0CCA */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdb38(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0CDC */
    d0 = set_word(d0, wv);
    if (sig(d0) < sig(w16(m, a4, 0xCEu)))                       /* 0C0CE6 bge */
        d0 = set_word(d0, w16(m, a4, 0xCEu));
    if (sig(d0) < sig(w16(m, a5, 0x1BC8u))) {                   /* 0C0CF0 bge */
        d0 = set_word(d0, w16(m, a5, 0x1BC8u));
        d0 = set_word(d0, (uint16_t)(d0 - 1u));                 /* 0C0CFA */
    }
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xEEu) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0D04 */
    if (sig(d1) < sig(d0)) {                                    /* 0C0D06 bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 0);
    }

    /* 0C0D30 */
    d0 = set_word(d0, (uint16_t)(w16(m, a4, 0xF6u) + w16(m, a4, 0xF8u)));
    d0 = set_word(d0, (uint16_t)(sig(d0) >> 1));                /* 0C0D38 */
    if ((int32_t)d3 > 0) {                                      /* 0C0D3A ble */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 - 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xFAu));                      /* 0C0D46 */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xFCu) - 4u));     /* 0C0D4A */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0D6C */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdd0e(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0D7E */
    d0 = set_word(d0, wv);
    if (sig(d0) > sig(w16(m, a4, 0xD0u)))                       /* 0C0D88 ble */
        d0 = set_word(d0, w16(m, a4, 0xD0u));
    d1 = set_word(d1, (uint16_t)(w16(m, a5, 0x1BC8u) + 0x140u)); /* 0C0D92 */
    if (sig(d0) > sig(d1)) d0 = set_word(d0, (uint16_t)d1);
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xF8u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0DA8 */
    if (sig(d0) < sig(d1)) {                                    /* 0C0DAA bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 1);
    }

    /* 0C0DD4 */
    d0 = set_word(d0, (uint16_t)(w16(m, a4, 0xEEu) + w16(m, a4, 0xF0u)));
    d0 = set_word(d0, (uint16_t)(sig(d0) >> 1));                /* 0C0DDC */
    if ((int32_t)d3 > 0) {                                      /* 0C0DDE ble */
        d0 = set_word(d0, (uint16_t)(d0 - (uint16_t)(d3 >> 16)));
        d0 = set_word(d0, (uint16_t)(d0 - 2u));
    }
    d1 = set_word(d1, w16(m, a4, 0xF2u));                      /* 0C0DEA */
    d2 = set_word(d2, (uint16_t)(w16(m, a4, 0xF4u) - 4u));     /* 0C0DEE */
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)(d1 + d2));
    d4 = set_word(d4, (uint16_t)(sig(d4) >> 1));
    d0 = set_word(d0, (uint16_t)(d0 + w16(m, a4, 0xA0u)));
    d1 = set_word(d1, (uint16_t)(d1 + w16(m, a4, 0xA4u)));
    d2 = set_word(d2, (uint16_t)(d2 + w16(m, a4, 0xA4u)));
    d4 = set_word(d4, (uint16_t)(d4 + w16(m, a4, 0xA4u)));
    d0 = set_word(d0, (uint16_t)(d0 + 2u));                     /* 0C0E10 */
    d1 = set_word(d1, (uint16_t)(d1 + 2u));
    d2 = set_word(d2, (uint16_t)(d2 - 2u));
    if (sig(d2) < sig(d1)) d1 = set_word(d1, (uint16_t)d2);
    h_bdd0e(m, a5, (uint16_t)d0, (uint16_t)d1, (uint16_t)d2, &wv, &a0);  /* 0C0E22 */
    d0 = set_word(d0, wv);
    if (sig(d0) > sig(w16(m, a4, 0xD0u)))                       /* 0C0E2C ble */
        d0 = set_word(d0, w16(m, a4, 0xD0u));
    d1 = set_word(d1, (uint16_t)(w16(m, a5, 0x1BC8u) + 0x140u)); /* 0C0E36 */
    if (sig(d0) > sig(d1)) d0 = set_word(d0, (uint16_t)d1);
    d1 = set_word(d1, (uint16_t)(w16(m, a4, 0xF0u) + w16(m, a4, 0xA0u)));
    d0 = set_word(d0, (uint16_t)(d0 - 2u));                     /* 0C0E4C */
    if (sig(d0) < sig(d1)) {                                    /* 0C0E4E bge */
        d1 = set_word(d1, (uint16_t)(d1 - d0));
        wr16(m, a4 + 0xA0u, (uint16_t)(w16(m, a4, 0xA0u) - d1));
        d0 = reflect_step(m, a4, d0, 1);
    }

tail:
    wr32(m, a4 + 0xCEu, 0x80007FFFu);                           /* 0C0E78 */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[7] = d7;
    r->a[0] = a0;
    r->a[7] = sp + 4u;                                          /* 0C0E80 rts */
    return 0;
}
