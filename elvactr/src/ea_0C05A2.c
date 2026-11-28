/* ROM 0x0C05A2: car/elevator step. Two "anim frame" lookups (0x0BFA46) give
 * the target delta, which is clamped to the car's travel range (+0xE6..+0xF0),
 * two corner points are probed against the solid map (0x0BD546), and the flags
 * collected in D5 dispatch through the 8-entry long table at 0x0C06FA. */
#include "ea_auto.h"

/* 0x0BFA46: look up the (X,Y) pair for frame `sel` in the record at A0:
 * table at A0+0xA, word per frame, high byte = X, low byte = Y (both signed);
 * with `flag` set the frame is mirrored inside the ((*(long*)A0)+2)+1)*16 wide
 * cell. Results are returned as two words (the ROM overwrites its stack arg). */
static void ea_bfa46(const f3mem_t *m, uint32_t a0, uint16_t sel, uint16_t flag,
                     uint16_t *ox, uint16_t *oy, uint32_t *od0, uint32_t *od1)
{
    uint16_t d0 = 0u, d1 = 0u, d2 = sel;

    if ((int16_t)d2 >= 0) {                                  /* 0x0BFA56 bmi */
        if ((uint16_t)d2 > 3u) {                             /* 0x0BFA5C bhi */
            /* keep sel */
        } else if (flag != 0u) {
            d2 = (uint16_t)(d2 ^ 1u);                        /* 0x0BFA64 eori.w */
        }
        d1 = rd16(m, a0 + 0x0Au + (uint32_t)(uint16_t)(d2 * 2u)); /* 0x0BFA68 */
        /* 0x0BFA6C bfexts d1{16:8},d0 : signed byte from bits 15..8 */
        d0 = (uint16_t)(int16_t)(int8_t)(uint8_t)(d1 >> 8);
        d1 = (uint16_t)(int16_t)(int8_t)(uint8_t)(d1 & 0xFFu);    /* 0x0BFA70 ext.w */
    }
    if (flag != 0u) {                                        /* 0x0BFA72 */
        uint16_t t;
        d0 = (uint16_t)(-(int16_t)d0);                       /* 0x0BFA78 neg.w */
        t = rd16(m, rd32(m, a0) + 2u);                       /* 0x0BFA7A ([a0],$2) */
        t = (uint16_t)(t + 1u);                              /* 0x0BFA80 */
        t = (uint16_t)(t << 4);                              /* 0x0BFA82 */
        t = (uint16_t)(t - 1u);                              /* 0x0BFA84 */
        d0 = (uint16_t)(d0 + t);                             /* 0x0BFA86 */
    }
    *ox = d0;
    *oy = d1;
    /* the helper's own D0/D1 as it leaves them: the caller's `move.w (a7)+` only
     * overwrites the low half, so these high halves are what the ROM leaves. */
    *od0 = (uint32_t)(int32_t)(int16_t)d0;
    *od1 = (uint32_t)(int32_t)(int16_t)d1;
}

/* BFTST <mem>{Do:w}: ext word 0x0801 is Do=1 (bit offset from D0, the caller's
 * x cell) with the low 5 bits 1 = width 1 (the field holds the width itself,
 * as the verified EEPROM bitfields in src/auto/eeprom_block.h show). Bit
 * strings count from the MSB of the addressed byte, so offset >= 8 walks on
 * in the tile bitmap. Returns non-zero when the bit is set (Z clear, which
 * `sne.b` turns into 0xFF). */
static int ea_bftest(const f3mem_t *m, uint32_t addr, uint32_t offset, unsigned width)
{
    for (unsigned i = 0; i < width; i++) {
        uint32_t o = offset + i;
        if ((rd8(m, addr + (o >> 3)) & (0x80u >> (o & 7u))) != 0u)
            return 1;
    }
    return 0;
}

/* CMP2 range test: the ROM's cmp2.w compares the value with the bounds word
 * pair as SIGNED integers, inclusive at both ends; C (bcs) means outside. */
static int ea_cmp2(uint16_t v, uint16_t lo, uint16_t hi)
{
    return (int16_t)v >= (int16_t)lo && (int16_t)v <= (int16_t)hi;
}

/* 0x0BD546 (with 0x0BD516 inlined): solid map test for the point (D0.w,D1.w).
 * Returns the Z flag the ROM leaves on return: 1 = free, 0 = blocked. The
 * callee's final D0 (moveq #0/#1/#2, or the untouched input) is written back. */
static int ea_bd546(const f3mem_t *m, uint32_t d0in, uint32_t d1in, uint32_t *d0out,
                    uint32_t *a0io)
{
    uint16_t x = (uint16_t)d0in, y = (uint16_t)d1in;
    int16_t sx, sy;
    uint32_t a0;
    uint16_t count, i;

    /* 0x0BD516: map cell bit for the scaled point */
    sx = (int16_t)(uint16_t)(x - rd16(m, 0x408998u));        /* 0x0BD516 sub.w */
    sx = (int16_t)(sx >> 2);                                 /* 0x0BD51A asr.w #2 */
    if ((uint16_t)sx >= rd16(m, 0x40899Cu))                  /* 0x0BD51C bcc */
        goto free;
    sy = (int16_t)(uint16_t)(y - rd16(m, 0x40899Au));        /* 0x0BD522 */
    sy = (int16_t)(sy >> 1);                                 /* 0x0BD526 asr.w #1 */
    if ((uint16_t)sy >= rd16(m, 0x40899Eu))                  /* 0x0BD528 bcc */
        goto free;
    a0 = rd32(m, 0x408994u);                                 /* 0x0BD530 */
    a0 = a0 + (uint32_t)(int32_t)(int16_t)rd16(m, a0 + (uint32_t)(int32_t)sy * 2u);
    /* 0x0BD538 bftst (a0){D0:1}: the x cell picks the bit */
    if (!ea_bftest(m, a0, (uint16_t)sx, 1u))
        goto free;
    /* 0x0BD54C movem.w (a7)+,d0-d1 : on the 68020 a word MOVEM load
     * sign-extends into the whole data register, Z=0 */
    *d0out = (uint32_t)(int32_t)(int16_t)d0in;
    return 0;

free:
    if ((int16_t)rd16(m, 0x41B818u) < 0) {                    /* 0x0BD552 bmi */
        *d0out = 0u;
        return 1;                                            /* 0x0BD59A moveq #0 */
    }
    count = rd16(m, 0x41B818u);                              /* 0x0BD55E */
    a0 = 0u;
    for (i = count; ; i--) {                                 /* 0x0BD56A dbra */
        uint32_t rec = rd32(m, 0x41B81Cu + (uint32_t)(count - i) * 4u); /* (a1)+ */
        *a0io = rec;                                                 /* 0x0BD56A movea.l (a1)+,a0 */
        if (ea_cmp2(x, rd16(m, rec + 0x34u), rd16(m, rec + 0x36u))) {   /* 0x0BD56C */
            if (ea_cmp2(y, rd16(m, rec + 0x3Cu), rd16(m, rec + 0x3Eu))) { /* 0x0BD574 */
                *d0out = 1u;
                return 0;                                    /* 0x0BD5A6 moveq #1 */
            }
        }
        if (rd32(m, rec + 0x24u) != 0u) {                    /* 0x0BD57C */
            if (ea_cmp2(x, rd16(m, rec + 0x24u), rd16(m, rec + 0x26u))) { /* 0x0BD582 */
                if (ea_cmp2(y, rd16(m, rec + 0x2Cu), rd16(m, rec + 0x2Eu))) { /* 0x0BD58A */
                    *d0out = 2u;
                    return 0;                                /* 0x0BD59E moveq #2 */
                }
            }
        }
        if (i == 0u)
            break;
    }
    *d0out = 0u;
    return 1;                                                /* 0x0BD59A moveq #0 */
}

/* 0x0C04AC: rebuild the car's pixel extents +0x8E..+0x94 from the frame
 * record at (*(long*)(A4+0x9C))+0x1C (two signed origin bytes, one word). */
static void ea_c04ac(const f3mem_t *m, uint32_t a4)
{
    uint32_t a0 = rd32(m, a4 + 0x9Cu) + 0x1Cu;
    uint16_t d0 = rd8(m, a0);                                 /* 0x0C04B4 */
    uint16_t d1 = rd8(m, a0 + 1u);                            /* 0x0C04B6 */
    uint16_t d2 = rd16(m, a0 + 2u);                           /* 0x0C04B8 */

    d0 = (uint16_t)(int16_t)(int8_t)(uint8_t)d0;              /* 0x0C04BA ext.w */
    d1 = (uint16_t)(int16_t)(int8_t)(uint8_t)d1;              /* 0x0C04BC ext.w */
    d2 = (uint16_t)(int16_t)(int8_t)(uint8_t)d2;              /* 0x0C04BE ext.w: the low
                                                               * byte of the word is what survives */
    d0 = (uint16_t)(d0 + d1);                                 /* 0x0C04C0 */
    d0 = (uint16_t)((int16_t)d0 >> 1);                        /* 0x0C04C2 asr.w #1 */
    if (rd16(m, a4 + 0x6Au) != 0u) {                          /* 0x0C04C4 */
        uint32_t p = rd32(m, a4 + 0x9Cu);                     /* 0x0C04CA */
        d1 = rd16(m, p + 2u);                                 /* 0x0C04D0 */
        d1 = (uint16_t)(d1 + 1u);
        d1 = (uint16_t)(d1 << 4);
        d1 = (uint16_t)(d1 - 1u);
        d0 = (uint16_t)(-(int16_t)d0);                        /* 0x0C04DA neg.w */
        d0 = (uint16_t)(d0 + d1);
    }
    wr16(m, a4 + 0x8Eu, d0);                                  /* 0x0C04DE */
    wr16(m, a4 + 0x90u, d2);                                  /* 0x0C04E2 */
    wr16(m, a4 + 0x92u, d0);                                  /* 0x0C04E6 */
    wr16(m, a4 + 0x94u, d2);                                  /* 0x0C04EA */
}

int Ea_0C05A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t d4 = r->d[4], d6 = r->d[6];
    uint32_t d5 = 0u;                          /* 0x0C05A2 moveq #$0,d5 */
    uint32_t a0 = r->a[0];
    uint32_t cd0, cd1;
    uint16_t x, y;
    uint16_t lim;
    int z;

    /* 0x0C05A4 move.w ([$9c,a4],$8),d4 */
    d4 = set_word(d4, rd16(m, rd32(m, a4 + 0x9Cu) + 8u));
    if ((uint16_t)d4 <= 3u && rd16(m, a4 + 0x6Au) != 0u)      /* 0x0C05AC..0x0C05B6 */
        d4 = set_word(d4, (uint16_t)((uint16_t)d4 ^ 1u));      /* 0x0C05B8 eori.w */

    /* 0x0C05BC..0x0C05D0: target lookup (A4+$AA, flag A4+$A8) */
    ea_bfa46(m, rd32(m, a4 + 0xAAu), (uint16_t)d4, rd16(m, a4 + 0xA8u), &x, &y, &cd0, &cd1);
    d0 = set_word(d0, x);                                     /* 0x0C05CC */
    /* 0x0C05CE move.w (a7)+,d1 writes the low word only; the high word is what
     * the helper itself left in D1 (the capture has 0x0000001E where the entry
     * carried 0x000C0000), i.e. the sign-extended Y byte. */
    d1 = cd1;

    /* 0x0C05D2..0x0C05E6: own lookup (A4+$9C, flag A4+$6A) */
    ea_bfa46(m, rd32(m, a4 + 0x9Cu), (uint16_t)d4, rd16(m, a4 + 0x6Au), &x, &y, &cd0, &cd1);
    d2 = set_word(d2, x);                                     /* 0x0C05E2 */
    d3 = set_word(d3, y);                                     /* 0x0C05E4 */

    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - (uint16_t)d2)); /* 0x0C05E8 sub.w */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - (uint16_t)d3)); /* 0x0C05EA sub.w */

    /* 0x0C05EC..0x0C0612: clamp the step into [min(+E6,+EE), max(+E8,+F0)] */
    d6 = set_word(d6, rd16(m, a4 + 0xE6u));                   /* 0x0C05EC */
    lim = rd16(m, a4 + 0xEEu);
    if ((int16_t)(uint16_t)d6 > (int16_t)lim)                 /* 0x0C05F0 ble */
        d6 = set_word(d6, lim);
    if ((int16_t)(uint16_t)d2 < (int16_t)(uint16_t)d6)        /* 0x0C05FA bge */
        d2 = set_word(d2, (uint16_t)d6);
    d6 = set_word(d6, rd16(m, a4 + 0xE8u));                   /* 0x0C0600 */
    lim = rd16(m, a4 + 0xF0u);
    if ((int16_t)(uint16_t)d6 < (int16_t)lim)                 /* 0x0C0604 bge */
        d6 = set_word(d6, lim);
    if ((int16_t)(uint16_t)d2 > (int16_t)(uint16_t)d6)        /* 0x0C060E ble */
        d2 = set_word(d2, (uint16_t)d6);

    wr16(m, a4 + 0x8Eu, (uint16_t)d2);                        /* 0x0C0614 */
    wr16(m, a4 + 0x90u, (uint16_t)d3);                        /* 0x0C0618 */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0xA0u))); /* 0x0C061C */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0xA4u))); /* 0x0C0620 */
    wr16(m, a4 + 0xA0u, (uint16_t)d0);                        /* 0x0C0624 */
    wr16(m, a4 + 0xA4u, (uint16_t)d1);                        /* 0x0C0628 */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d2)); /* 0x0C062C */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 + (uint16_t)d3)); /* 0x0C062E */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - 1u));          /* 0x0C0630 */

    z = ea_bd546(m, d0, d1, &d0, &a0);                         /* 0x0C0632 */
    d0 = (d0 & 0xFFFFFF00u) | (z ? 0x00u : 0xFFu);             /* 0x0C0638 sne.b */
    wr8(m, a4 + 0x8Cu, (uint8_t)(d0 & 0xFFu));                 /* 0x0C063A */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 & 4u));          /* 0x0C063E */
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 | (uint16_t)d0)); /* 0x0C0642 */

    if ((uint16_t)d4 <= 3u)                                    /* 0x0C0644 bhi */
        d4 = set_word(d4, (uint16_t)((uint16_t)d4 ^ 1u));      /* 0x0C064A */

    /* 0x0C064E..0x0C0662: own lookup again (mirrored frame) */
    ea_bfa46(m, rd32(m, a4 + 0x9Cu), (uint16_t)d4, rd16(m, a4 + 0x6Au), &x, &y, &cd0, &cd1);
    d2 = set_word(d2, x);                                      /* 0x0C065E */
    d3 = set_word(d3, y);                                      /* 0x0C0660 */

    /* 0x0C0664..0x0C068A: the same clamp */
    d6 = set_word(d6, rd16(m, a4 + 0xE6u));                     /* 0x0C0664 */
    lim = rd16(m, a4 + 0xEEu);
    if ((int16_t)(uint16_t)d6 > (int16_t)lim)
        d6 = set_word(d6, lim);
    if ((int16_t)(uint16_t)d2 < (int16_t)(uint16_t)d6)
        d2 = set_word(d2, (uint16_t)d6);
    d6 = set_word(d6, rd16(m, a4 + 0xE8u));                     /* 0x0C0678 */
    lim = rd16(m, a4 + 0xF0u);
    if ((int16_t)(uint16_t)d6 < (int16_t)lim)
        d6 = set_word(d6, lim);
    if ((int16_t)(uint16_t)d2 > (int16_t)(uint16_t)d6)
        d2 = set_word(d2, (uint16_t)d6);

    wr16(m, a4 + 0x92u, (uint16_t)d2);                          /* 0x0C068C */
    wr16(m, a4 + 0x94u, (uint16_t)d3);                          /* 0x0C0690 */
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 + rd16(m, a4 + 0xA0u))); /* 0x0C0694 */
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 + rd16(m, a4 + 0xA4u))); /* 0x0C0698 */
    d1 = set_word(d1, (uint16_t)d3);                            /* 0x0C069C */
    d0 = set_word(d0, (uint16_t)d2);                            /* 0x0C069E */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - 1u));            /* 0x0C06A0 */

    z = ea_bd546(m, d0, d1, &d0, &a0);                          /* 0x0C06A2 */
    d0 = (d0 & 0xFFFFFF00u) | (z ? 0x00u : 0xFFu);              /* 0x0C06A8 sne.b */
    wr8(m, a4 + 0x8Du, (uint8_t)(d0 & 0xFFu));                  /* 0x0C06AA */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 & 2u));           /* 0x0C06AE */
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 | (uint16_t)d0)); /* 0x0C06B2 */

    d0 = set_word(d0, rd16(m, a4 + 0x94u));                     /* 0x0C06B4 */
    d0 = (d0 & 0xFFFFFF00u) |                            /* 0x0C06BC slt.b d0 */
         (((int16_t)(uint16_t)d0 < (int16_t)rd16(m, a4 + 0x90u)) ? 0xFFu : 0x00u);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 & 1u));           /* 0x0C06BE */
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 | (uint16_t)d0)); /* 0x0C06C2 */

    /* 0x0C06C4 tst.w ([$72,a4],$c) */
    if (rd16(m, rd32(m, a4 + 0x72u) + 0xCu) != 0u) {
        ea_c04ac(m, a4);                                        /* 0x0C06CE */
        a0 = rd32(m, a4 + 0x9Cu) + 0x20u;                       /* 0x0C04AC leaves A0 there */
        d0 = set_word(d0, rd16(m, a4 + 0x8Eu));                 /* 0x0C06D2 */
        d1 = set_word(d1, rd16(m, a4 + 0x90u));                 /* 0x0C06D6 */
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 + rd16(m, a4 + 0xA0u))); /* 0x0C06DA */
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 + rd16(m, a4 + 0xA4u))); /* 0x0C06DE */
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 - 1u));        /* 0x0C06E2 */
        z = ea_bd546(m, d0, d1, &d0, &a0);                       /* 0x0C06E4 */
        if (!z)
            goto L079A;                                          /* 0x0C06EA bne */
        goto L071A;                                              /* 0x0C06EE bra */
    }

    switch ((uint16_t)d5 & 7u) {                                 /* 0x0C06F2 jmp */
    case 0:
    case 1:
        goto L071A;                     /* table[0] = table[1] = 0x0C071A */
    case 7:
    case 2:
    case 3:
        goto L0792;                     /* 0x0C0792 (table[2,3,7]) */
    default:
        goto L079A;                     /* 0x0C079A (table[4,5,6]) */
    }

L071A:
    /* 0x0C071A move.l ([$ae,a4],$c),d0 ; andi.l #$400 */
    d0 = rd32(m, rd32(m, a4 + 0xAEu) + 0xCu) & 0x400u;
    if (d0 != 0u) {                                              /* 0x0C0728 beq */
        d2 = set_word(d2, rd16(m, a4 + 0xA0u));                  /* 0x0C072A */
        d3 = set_word(d3, rd16(m, a4 + 0xA4u));                  /* 0x0C072E */
        d2 = set_word(d2, (uint16_t)((uint16_t)d2 + rd16(m, a4 + 0x8Eu)));
        d3 = set_word(d3, (uint16_t)((uint16_t)d3 + rd16(m, a4 + 0x90u)));
        d1 = set_word(d1, (uint16_t)d3);                         /* 0x0C073A */
        d0 = set_word(d0, (uint16_t)d2);                         /* 0x0C073C */
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 + 2u));         /* 0x0C073E */
        z = ea_bd546(m, d0, d1, &d0, &a0);                        /* 0x0C0740 */
        if (!z)
            goto L0760;                                          /* 0x0C0746 bne */
        d2 = set_word(d2, (uint16_t)(rd16(m, a4 + 0xA0u) + rd16(m, a4 + 0x92u)));
        d1 = set_word(d1, (uint16_t)d3);                         /* 0x0C0750 */
        d0 = set_word(d0, (uint16_t)d2);                         /* 0x0C0752 */
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 + 2u));         /* 0x0C0754 */
        z = ea_bd546(m, d0, d1, &d0, &a0);                        /* 0x0C0756 */
        if (!z)
            goto L0760;                                          /* 0x0C075C bne */
    }

    /* 0x0C0766 bftst ([$9c,a4],$20){15:1} : bit 0 of the word */
    if ((rd16(m, rd32(m, a4 + 0x9Cu) + 0x20u) & 1u) == 0u) {
        d0 = rd32(m, a4 + 0xC2u);                                /* 0x0C0772 */
        wr32(m, a4 + 0xA0u, rd32(m, a4 + 0xA0u) + d0);            /* 0x0C0776 */
        wr32(m, a4 + 0xC6u, rd32(m, a4 + 0xC6u) + 0x2000u);       /* 0x0C077A */
        d0 = rd32(m, a4 + 0xC6u);                                 /* 0x0C0782 */
        wr32(m, a4 + 0xA4u, rd32(m, a4 + 0xA4u) + d0);            /* 0x0C0786 */
    }
    /* 0x0C078A move.w #$ffff,$86(a4) ; 0x0C0790 rts */
    wr16(m, a4 + 0x86u, 0xFFFFu);
    goto Lout;

L0760:
    wr16(m, a4 + 0x86u, 0u);                                     /* 0x0C0760 */
    goto Lout;                                                   /* 0x0C0764 rts */

L0792:
    wr16(m, a4 + 0x86u, 1u);                                     /* 0x0C0792 */
    goto L079E;                                                  /* 0x0C0798 bra */

L079A:
    wr16(m, a4 + 0x86u, 0u);                                     /* 0x0C079A */

L079E:                                                           /* 0x0C079E */
    d3 = 0xFFFFFFFAu;                                            /* 0x0C079E moveq #$fa */
    d2 = set_word(d2, rd16(m, a4 + 0xA0u));                       /* 0x0C07A0 */
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 + rd16(m, a4 + 0xA4u))); /* 0x0C07A4 */
    d1 = set_word(d1, rd16(m, a4 + 0x94u));                       /* 0x0C07A8 */
    d0 = set_word(d0, rd16(m, a4 + 0x92u));                       /* 0x0C07AC */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d2));    /* 0x0C07B0 */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 + (uint16_t)d3));    /* 0x0C07B2 */
    z = ea_bd546(m, d0, d1, &d0, &a0);                              /* 0x0C07B4 */
    if (z)
        goto Lout;                                                 /* 0x0C07BA beq */
    d1 = set_word(d1, rd16(m, a4 + 0x90u));                        /* 0x0C07BE */
    d0 = set_word(d0, rd16(m, a4 + 0x8Eu));                        /* 0x0C07C2 */
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d2));     /* 0x0C07C6 */
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 + (uint16_t)d3));     /* 0x0C07C8 */
    z = ea_bd546(m, d0, d1, &d0, &a0);                               /* 0x0C07CA */
    if (!z)
        goto L071A;                                                 /* 0x0C07D0 bne */

Lout:                                                              /* 0x0C07D4 rts */
    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[5] = d5;
    r->d[6] = d6;
    r->a[0] = a0;
    r->a[7] = sp + 4u;
    return 0;
}
