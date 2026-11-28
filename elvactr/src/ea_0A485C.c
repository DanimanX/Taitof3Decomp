/* ROM 0x0A485C: credit/insert-coin line ticker: every 0x3C frames it flips the
 * blink byte at A5-0x60FE and, from the coin/credit state, draws
 * "PUSH START BUTTON" / "INSERT COIN" / "TO CONTINUE" / "TO START" plus the
 * credit count through the text helpers 0x0069F4 and 0x006AA8. */
#include "ea_auto.h"

/* 0x000B00: D0.b = show-line flag built from A5-0x7F5E/-0x7F6E/-0x7F70. */
static void call_000B00(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t d0;

    /* 0x000B00 move.w -$7f5e(a5), d0 */
    d0 = (r->d[0] & 0xFFFF0000u) | (uint16_t)rd16(m, a5 - 0x7F5Eu);
    /* 0x000B04 beq $b16 */
    if ((d0 & 0xFFFFu) == 0) {
        /* 0x000B16 tst.w -$7f70(a5) ; seq.b d0 */
        if (rd16(m, a5 - 0x7F70u) == 0)
            d0 = (d0 & 0xFFFFFF00u) | 0xFFu;
        else
            d0 &= 0xFFFFFF00u;
    } else {
        /* 0x000B06 sub.w -$7f6e(a5), d0 */
        uint16_t v = (uint16_t)((uint16_t)d0 - rd16(m, a5 - 0x7F6Eu));
        d0 = (d0 & 0xFFFF0000u) | v;
        /* 0x000B0A ble $b12 ; 0x000B0C tst.w -$7f70(a5) ; 0x000B10 beq $b14 */
        if ((int16_t)v <= 0 || rd16(m, a5 - 0x7F70u) != 0)
            d0 = 0u;                                   /* 0x000B12 moveq #0 */
    }

    r->d[0] = d0;
    r->a[7] += 4u;                                     /* 0x000B1C rts */
}

/* 0x006AA8: write (count) decimal digits of the 32-bit value into buffer,
 * right to left in word steps; D0 returns buffer + count*2. Attribute word
 * arg's low 6 bits * 2 << 8 rides in the high byte of every digit. */
static void call_006AA8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t sp = r->a[7];
    uint32_t a6 = sp - 4u;
    uint32_t a0 = rd32(m, sp + 10u);                   /* arg2 $e(a6) */
    uint32_t v = rd32(m, sp + 4u);                     /* arg0 $8(a6) */
    uint16_t d1 = rd16(m, sp + 8u);                    /* arg1 $c(a6) */
    uint16_t d2 = (uint16_t)((rd16(m, sp + 14u) & 0x003Fu) << 1) << 8; /* arg3 $12(a6) */
    uint32_t d3, d4, d0, d5;

    wr32(m, a6, r->a[6]);                              /* 0x006AA8 link.w a6,#0 */
    /* 0x006AAC movem.l d1-d5/a0, -(a7) */
    wr32(m, a6 - 4u, r->d[1]);
    wr32(m, a6 - 8u, r->d[2]);
    wr32(m, a6 - 12u, r->d[3]);
    wr32(m, a6 - 16u, r->d[4]);
    wr32(m, a6 - 20u, r->d[5]);
    wr32(m, a6 - 24u, r->a[0]);

    /* 0x006AD0 move.l a0, d0 ; 0x006AD2/0x006AD4 d4 = zero-extended d3.w */
    d4 = v & 0xFFFFu;
    /* 0x006AD6 clr.w d3 ; 0x006AD8 swap d3 -> d3.w = value high word */
    d3 = (v >> 16) & 0xFFFFu;

    /* 0x006AC8 move.w d1,d0 ; lsl.w #1,d0 ; adda.w d0,a0 ; subq.w #1,d1 */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)(d1 << 1);
    d1 = (uint16_t)(d1 - 1u);
    d0 = a0;
    d5 = 0u;

    for (;;) {
        /* 0x006ADA tst.w d3 ; 0x006ADC bne $6af4 */
        if (d3 == 0u) {
            /* 0x006AE0 tst.w d4 ; bne $6af4 */
            if (d4 == 0u) {
                /* 0x006AE6 cmpa.l d0,a0 ; beq $6b10 */
                if (a0 == d0) {
                    /* 0x006AEC move.b #$30,d2 ; bra.w $6b14 */
                    d2 = (uint16_t)((d2 & 0xFF00u) | 0x30u);
                    goto write_digit;
                }
            }
        }

        /* 0x006AF4 divu.w #$a, d3 */
        {
            uint32_t q = d3 / 10u, rem = d3 % 10u;
            d3 = (rem << 16) | (q & 0xFFFFu);
        }
        /* 0x006AF8 move.l d3,d5 ; 0x006AFA clr.w d5 ; 0x006AFC add.l d5,d4 */
        d5 = d3 & 0xFFFF0000u;
        d4 = d4 + d5;
        /* 0x006AFE swap d3 ; 0x006B00 clr.w d3 ; 0x006B02 swap d3 */
        d3 = ((d3 << 16) | (d3 >> 16)) & 0xFFFF0000u;
        d3 = (d3 << 16) | (d3 >> 16);
        /* 0x006B04 divu.w #$a, d4 */
        {
            uint32_t q = d4 / 10u, rem = d4 % 10u;
            d4 = ((rem & 0xFFFFu) << 16) | (q & 0xFFFFu);
        }
        /* 0x006B08 swap d4 */
        d4 = (d4 << 16) | (d4 >> 16);
        /* 0x006B0A move.b d4,d2 */
        d2 = (uint16_t)((d2 & 0xFF00u) | (d4 & 0xFFu));
        /* 0x006B0C clr.w d4 ; 0x006B0E swap d4 */
        d4 = (d4 & 0xFFFF0000u);
        d4 = (d4 << 16) | (d4 >> 16);
        /* 0x006B10 addi.b #$30, d2 */
        d2 = (uint16_t)(d2 + 0x30u);

    write_digit:
        /* 0x006B14 move.w d2, -(a0) */
        a0 -= 2u;
        wr16(m, a0, d2);
        /* 0x006B16 dbra d1, $6ada */
        if (d1 == 0u) {
            d1 = 0xFFFFu;
            break;
        }
        d1 = (uint16_t)(d1 - 1u);
    }

    r->d[0] = d0;                                      /* D0 survives the rts */
    r->d[3] = d3;
    r->d[4] = d4;
    r->d[5] = d5;

    /* 0x006B1A movem.l (a7)+, d1-d5/a0 */
    r->d[1] = rd32(m, a6 - 4u);
    r->d[2] = rd32(m, a6 - 8u);
    r->d[3] = rd32(m, a6 - 12u);
    r->d[4] = rd32(m, a6 - 16u);
    r->d[5] = rd32(m, a6 - 20u);
    r->a[0] = rd32(m, a6 - 24u);
    /* 0x006B1E unlk a6 */
    r->a[6] = rd32(m, a6);
    /* 0x006B20 rts */
    r->a[7] = sp + 4u;
}

/* 0x0069F4 (string copy with 0x0A attribute words): push the attr, the
 * destination, the ROM string and the return address, then call it. */
static void call_69F4(const f3mem_t *m, f3regs_t *r,
                      uint32_t dst, uint32_t str, uint16_t attr, uint32_t ret)
{
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_long(m, r, str);
    push_long(m, r, ret);
    Ea_0069F4(m, r);
    r->a[7] += 0xAu;
}

/* 0x006AA8 call: args (d3.l value, 1.w count, d0.l buffer, 5.w attribute). */
static void call_6aa8(const f3mem_t *m, f3regs_t *r,
                      uint32_t value, uint32_t dst, uint32_t ret)
{
    push_word(m, r, 5u);
    push_long(m, r, dst);
    push_word(m, r, 1u);
    push_long(m, r, value);
    push_long(m, r, ret);
    call_006AA8(m, r);
    r->a[7] += 0xCu;
}

int Ea_0A485C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2];
    uint32_t d3 = r->d[3], d3_saved = r->d[3];
    uint16_t w;

    /* 0x0A485C subq.w #1, -$6100(a5) */
    w = (uint16_t)(rd16(m, a5 - 0x6100u) - 1u);
    wr16(m, a5 - 0x6100u, w);
    /* 0x0A4860 bgt.b $a486e */
    if ((int16_t)w > 0)
        goto L486E;

    /* 0x0A4862 move.w #$3c, -$6100(a5) */
    wr16(m, a5 - 0x6100u, 0x003Cu);
    /* 0x0A4868 bchg.b #$0, -$60fe(a5) */
    wr8(m, a5 - 0x60FEu, (uint8_t)(rd8(m, a5 - 0x60FEu) ^ 0x01u));

L486E:
    /* 0x0A486E tst.w -$60fe(a5) ; bne.w $a49e2 */
    if (rd16(m, a5 - 0x60FEu) != 0)
        goto L49E2;

    /* 0x0A4876 tst.b -$7f2d(a5) ; bne.b $a48ae */
    if (rd8(m, a5 - 0x7F2Du) != 0)
        goto L48AE;

    /* 0x0A487C tst.l d1 ; bne.b $a488c */
    if (d1 != 0u)
        goto L488C;

    /* 0x0A4880 jsr $ad6.l */
    push_long(m, r, 0x0A4886u);
    Ea_000AD6(m, r);
    d0 = r->d[0];
    /* 0x0A4886 tst.b d0 ; bne.b $a48c6 */
    if ((d0 & 0xFFu) != 0u)
        goto L48C6;
    /* 0x0A488A bra.b $a48ae */
    goto L48AE;

L488C:
    /* 0x0A488C tst.b -$7f2e(a5) ; bne.b $a4894 */
    if (rd8(m, a5 - 0x7F2Eu) == 0u) {
        /* 0x0A4892 rts */
        goto done;
    }
    /* 0x0A4894 jsr $b00.l */
    push_long(m, r, 0x0A489Au);
    call_000B00(m, r);
    d0 = r->d[0];
    /* 0x0A489A tst.b d0 ; bne.b $a48c6 */
    if ((d0 & 0xFFu) != 0u)
        goto L48C6;
    /* 0x0A489E cmpi.w #$2, $1ffffe.l ; bne.b $a48ae */
    if (rd16(m, 0x1FFFFEu) != 2u)
        goto L48AE;
    /* 0x0A48A8 move.w #$1, -$7850(a5) */
    wr16(m, a5 - 0x7850u, 1u);

L48AE:
    /* 0x0A48AE..0x0A48C4 */
    call_69F4(m, r, d2, 0x000A4E4Cu, 5u, 0x0A48C0u);
    goto done;

L48C6:
    /* 0x0A48C6 cmpi.w #$2, $1ffffe.l ; bne.w $a49ca */
    if (rd16(m, 0x1FFFFEu) != 2u)
        goto L49CA;
    /* 0x0A48D2 cmpi.w #$1, -$7f5c(a5) ; beq.w $a49ca */
    if (rd16(m, a5 - 0x7F5Cu) == 1u)
        goto L49CA;

    /* 0x0A48DC movem.l d3, -(a7) */
    d3_saved = d3;
    push_long(m, r, d3);

    /* 0x0A48E0 ext.w d0 ; blt.w $a497e */
    d0 = (uint32_t)(int32_t)(int16_t)(d0 & 0xFFFFu);
    r->d[0] = d0;
    if ((int32_t)d0 < 0)
        goto L497E;

    /* 0x0A48E6 move.w d0, d3 */
    d3 = (d3 & 0xFFFF0000u) | (d0 & 0xFFFFu);

    /* 0x0A48E8..0x0A48FE */
    call_69F4(m, r, d2, 0x000A4EA1u, 5u, 0x0A48FAu);
    d0 = r->d[0];
    /* 0x0A48FE cmp.w -$7f5e(a5), d3 ; beq.w $a491c */
    if ((uint16_t)d3 != rd16(m, a5 - 0x7F5Eu))
        call_69F4(m, r, d2, 0x000A4EA9u, 5u, 0x0A4918u);
    d0 = r->d[0];

    /* 0x0A491C..0x0A492E */
    call_6aa8(m, r, d3, d0, 0x0A492Eu);
    d0 = r->d[0];
    /* 0x0A4932 cmpi.w #$1, d3 ; beq.b $a4950 */
    if ((uint16_t)d3 == 1u) {
        /* 0x0A4950..0x0A4962 */
        call_69F4(m, r, d0, 0x000A4EB5u, 5u, 0x0A4962u);
    } else {
        /* 0x0A4938..0x0A494A */
        call_69F4(m, r, d0, 0x000A4EAEu, 5u, 0x0A494Au);
    }
    d0 = r->d[0];
    /* 0x0A4966..0x0A4978 */
    call_69F4(m, r, d0, 0x000A4ED1u, 5u, 0x0A4978u);
    goto L49C4;

L497E:
    /* 0x0A497E move.w -$7f5c(a5), d3 */
    d3 = (d3 & 0xFFFF0000u) | (uint16_t)rd16(m, a5 - 0x7F5Cu);
    /* 0x0A4982..0x0A4994 */
    call_69F4(m, r, d2, 0x000A4EA1u, 5u, 0x0A4994u);
    d0 = r->d[0];
    /* 0x0A4998..0x0A49AA */
    call_6aa8(m, r, d3, d0, 0x0A49AAu);
    d0 = r->d[0];
    /* 0x0A49AE..0x0A49C0 */
    call_69F4(m, r, d0, 0x000A4EAEu, 5u, 0x0A49C0u);

L49C4:
    /* 0x0A49C4 movem.l (a7)+, d3 */
    d3 = d3_saved;
    goto done;

L49CA:
    /* 0x0A49CA..0x0A49E0 */
    call_69F4(m, r, d2, 0x000A4E5Eu, 5u, 0x0A49DCu);
    goto done;

L49E2:
    /* 0x0A49E2 tst.l d1 ; bne.b $a49fe */
    if (d1 != 0u)
        goto L49FE;
    /* 0x0A49E6..0x0A49FC */
    call_69F4(m, r, d2, 0x000A4E7Eu, 5u, 0x0A49F8u);
    goto done;

L49FE:
    /* 0x0A49FE tst.b -$7f2e(a5) ; beq.b $a4a1a */
    if (rd8(m, a5 - 0x7F2Eu) == 0u)
        goto done;
    /* 0x0A4A04..0x0A4A16 */
    call_69F4(m, r, d2, 0x000A4E6Cu, 5u, 0x0A4A16u);

done:
    r->d[3] = d3;
    r->a[7] = sp + 4u;                                 /* final rts */
    return 0;
}
