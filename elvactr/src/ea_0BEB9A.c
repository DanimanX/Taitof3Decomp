/* ROM 0x0BEB9A: continue-screen line/timer ticker for the panel object at A4.
 * Rotates the blink word at +0x8DA; when the object is on the "off" half of
 * the blink (or has a negative +0x02) it runs the whole insert-coin/timer
 * update: it marks the line byte at A5+0x1C8A, pushes 18-byte display-list
 * entries through the A5+0x1CF0/A5+0x1CFA counters and the A5+0x1D00/0x1D14
 * write pointers, probes the tile map (0x0BD516/0x0BD546/0x0BD5D8/0x0BD790),
 * draws the countdown digits into GFX RAM 0x61C000 and ages the timer at
 * +0x5E. */
#include "ea_auto.h"

/* cmp2.w (ea),Dn: true when v is outside the signed word bounds {lo,hi}. */
static int outside(const f3mem_t *m, uint32_t ea, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, ea), hi = (int16_t)rd16(m, ea + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* asr.w: arithmetic right shift of a 16-bit word (rounds towards -inf). */
static uint16_t asr16(uint16_t v, unsigned n)
{
    int16_t s = (int16_t)v;
    return s >= 0 ? (uint16_t)(s >> n) : (uint16_t)~((uint16_t)(~s) >> n);
}

/* divu.w: 32-bit dividend / 16-bit divisor -> quotient in the low word and
 * remainder in the high word; an overflow leaves the destination unchanged. */
static uint32_t divu16(uint32_t d, uint16_t div)
{
    uint32_t q = d / div, r = d % div;
    if (q > 0xFFFFu)
        return d;
    return ((r & 0xFFFFu) << 16) | (q & 0xFFFFu);
}

/* 18-byte display entry pushed backwards: long tag, 0, 0, w8, w10, w12, w14, 0 */
static void emit_entry(const f3mem_t *m, uint32_t a5, uint32_t tag,
                       uint16_t w8, uint16_t w10, uint16_t w12)
{
    uint32_t a0;

    if (rd16(m, a5 + 0x1CF0u) == 0)
        return;
    a0 = rd32(m, a5 + 0x1D00u);
    a0 -= 2u; wr16(m, a0, 0u);
    a0 -= 2u; wr16(m, a0, 0u);
    a0 -= 2u; wr16(m, a0, w12);
    a0 -= 2u; wr16(m, a0, w10);
    a0 -= 2u; wr16(m, a0, w8);
    a0 -= 2u; wr16(m, a0, 0u);
    a0 -= 2u; wr16(m, a0, 0u);
    a0 -= 4u; wr32(m, a0, tag);
    wr16(m, a5 + 0x1CF0u, (uint16_t)(rd16(m, a5 + 0x1CF0u) - 1u));
    wr32(m, a5 + 0x1D00u, a0);
}

/* 0x006AA8: write `count` decimal digits of `value` right to left in word steps
 * into dst; the attribute word's low six bits ride in the high byte. Returns
 * dst + count*2 (D0 in the ROM). */
static uint32_t write_digits(const f3mem_t *m, uint32_t value, uint32_t dst,
                             uint16_t count, uint16_t attr)
{
    uint32_t a0 = dst + (uint32_t)(int32_t)(int16_t)(uint16_t)(count << 1);
    uint32_t d0 = a0;
    uint32_t d4 = value & 0xFFFFu;
    uint32_t d3 = (value >> 16) & 0xFFFFu;
    uint16_t d2 = (uint16_t)(((attr & 0x003Fu) << 1) << 8);
    uint16_t d1 = (uint16_t)(count - 1u);

    for (;;) {
        if (d3 == 0u && d4 == 0u && a0 == d0) {
            d2 = (uint16_t)((d2 & 0xFF00u) | 0x30u);
        } else {
            uint32_t q, r;
            q = d3 / 10u; r = d3 % 10u;
            d3 = ((r & 0xFFFFu) << 16) | (q & 0xFFFFu);
            d4 += d3 & 0xFFFF0000u;
            d3 = (d3 << 16) | (d3 >> 16);
            d3 = (d3 << 16) | (d3 >> 16);
            q = d4 / 10u; r = d4 % 10u;
            d4 = ((r & 0xFFFFu) << 16) | (q & 0xFFFFu);
            d4 = (d4 << 16) | (d4 >> 16);
            d2 = (uint16_t)((d2 & 0xFF00u) | (d4 & 0xFFu));
            d4 &= 0xFFFF0000u;
            d4 = (d4 << 16) | (d4 >> 16);
            d2 = (uint16_t)(d2 + 0x30u);
        }
        a0 -= 2u;
        wr16(m, a0, d2);
        if (d1 == 0u)
            break;
        d1--;
    }
    return d0;
}

/* 0x006AA8 call: (value.l, count.w, dst.l, attr.w). */
static void call_6AA8(const f3mem_t *m, f3regs_t *r, uint32_t value, uint32_t dst,
                      uint16_t count, uint16_t attr)
{
    push_word(m, r, attr);
    push_long(m, r, dst);
    push_word(m, r, count);
    push_long(m, r, value);
    push_long(m, r, 0u);            /* return address (never read back) */
    r->d[0] = write_digits(m, value, dst, count, attr);
    r->a[7] += 0xCu;
}

/* 0x0BD516: probe one tile: D0.b = 0xFF when bit 7 of the tile byte is set. */
static void h_BD516(const f3mem_t *m, uint32_t a5, uint32_t *d0p, uint32_t *d1p,
                    uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, a0 = *a0p;
    uint16_t v;

    v = (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x0998u));
    v = asr16(v, 2);
    d0 = set_word(d0, v);
    if ((uint16_t)d0 >= rd16(m, a5 + 0x099Cu)) {           /* 0x0BD520 bcc */
        d0 = 0u;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return;
    }
    v = (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x099Au));
    v = asr16(v, 1);
    d1 = set_word(d1, v);
    if ((uint16_t)d1 >= rd16(m, a5 + 0x099Eu)) {           /* 0x0BD52C bcc */
        d0 = 0u;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return;
    }
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;         /* 0x0BD52E ext.l */
    a0 = rd32(m, a5 + 0x0994u);                            /* 0x0BD530 */
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d1 * 2u);
    /* 0x0BD538 bftst (a0){0:1} ; sne.b d0 ; ext.w d0 */
    d0 = set_word(d0, (rd8(m, a0) & 0x80u) ? 0xFFFFu : 0x0000u);

    *d0p = d0; *d1p = d1; *a0p = a0;
}

/* 0x0BD546: find the object under (D0,D1); 0 none, 1 body hit, 2 extra box. */
static uint16_t h_BD546(const f3mem_t *m, uint32_t a5, uint32_t *d0p, uint32_t *d1p,
                        uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, a0 = *a0p;
    uint16_t sd0 = (uint16_t)d0, sd1 = (uint16_t)d1;
    uint32_t d2, a1;

    h_BD516(m, a5, &d0, &d1, &a0);
    if ((uint16_t)d0 != 0u) {                              /* 0x0BD550 bne */
        d0 = (d0 & 0xFFFF0000u) | sd0;
        d1 = (d1 & 0xFFFF0000u) | sd1;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return (uint16_t)d0;
    }
    d0 = (d0 & 0xFFFF0000u) | sd0;                         /* 0x0BD54C movem.w */
    d1 = (d1 & 0xFFFF0000u) | sd1;
    if ((int16_t)rd16(m, 0x41B818u) < 0) {                 /* 0x0BD558 bmi */
        d0 = 0u;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return 0u;
    }
    d2 = rd16(m, 0x41B818u);
    a1 = 0x41B81Cu;
    for (;;) {
        a0 = rd32(m, a1);
        a1 += 4u;
        if (!outside(m, a0 + 0x34u, (uint16_t)d0)) {
            if (!outside(m, a0 + 0x3Cu, (uint16_t)d1)) {
                d0 = 1u;
                break;
            }
        }
        if (rd32(m, a0 + 0x24u) != 0u) {
            if (!outside(m, a0 + 0x24u, (uint16_t)d0)) {
                if (!outside(m, a0 + 0x2Cu, (uint16_t)d1)) {
                    d0 = 2u;
                    break;
                }
            }
        }
        if (d2 == 0u) {
            d0 = 0u;
            break;
        }
        d2--;
    }
    *d0p = d0; *d1p = d1; *a0p = a0;
    return (uint16_t)d0;
}

/* 0x0BD5D8: tile at (D0,D1); D0.w = 0x8000 with A0 = 0x0BD5AE when off map. */
static uint16_t h_BD5D8(const f3mem_t *m, uint32_t a5, uint32_t *d0p, uint32_t *d1p,
                        uint32_t *d2p, uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = *d2p, a0 = *a0p;
    uint16_t sd2 = (uint16_t)d2;

    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x0998u)));
    d0 = set_word(d0, asr16((uint16_t)d0, 2));
    if ((uint16_t)d0 >= rd16(m, a5 + 0x099Cu))
        goto oob;
    d2 = set_word(d2, (uint16_t)d1);
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x099Au)));
    d2 = set_word(d2, asr16((uint16_t)d2, 1));
    if ((uint16_t)d2 >= rd16(m, a5 + 0x099Eu))
        goto oob;
    a0 = rd32(m, a5 + 0x0984u);
    d0 = set_word(d0, rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d0 * 2u));
    d0 = set_word(d0, (uint16_t)(-(int16_t)(uint16_t)d0));
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d0 * 2u - 2u);
    a0 -= 4u;
    do {
        a0 += 4u;
    } while ((int16_t)d1 < (int16_t)rd16(m, a0));
    d0 = set_word(d0, rd16(m, a0));
    a0 += 2u;
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0);
    d2 = (d2 & 0xFFFF0000u) | sd2;
    *d0p = d0; *d1p = d1; *d2p = d2; *a0p = a0;
    return (uint16_t)d0;

oob:
    d0 = set_word(d0, 0x8000u);
    a0 = 0x000BD5AEu;
    d2 = (d2 & 0xFFFF0000u) | sd2;
    *d0p = d0; *d1p = d1; *d2p = d2; *a0p = a0;
    return (uint16_t)d0;
}

/* 0x0BD790: collect a column of tiles between two x coordinates; the results
 * are handed back through the caller's argument words (D0.w, A0). */
static void h_BD790(const f3mem_t *m, uint32_t a5, uint16_t arg_y, uint32_t arg_x2,
                    uint16_t arg_xlo, uint16_t *out_w, uint32_t *out_a0)
{
    uint32_t d0, d1, d2, d3, a0, a1, a2;

    d0 = arg_y;
    d0 = (uint16_t)(d0 - rd16(m, a5 + 0x099Au));
    d0 = asr16((uint16_t)d0, 1);
    if ((uint16_t)d0 >= rd16(m, a5 + 0x099Eu)) {           /* 0x0BD7A2 bcc */
        *out_w = 0x7FFFu;
        *out_a0 = 0x000BD5AEu;
        return;
    }
    d2 = arg_xlo;
    d2 = (uint16_t)(d2 - rd16(m, a5 + 0x0998u));
    if ((int16_t)d2 < 0) {
        d2 = 0u;
    } else {
        d2 = asr16((uint16_t)d2, 2);
        if ((uint16_t)d2 >= rd16(m, a5 + 0x099Cu)) {
            d2 = (uint16_t)(rd16(m, a5 + 0x099Cu) - 1u);
        }
    }
    d1 = (uint16_t)(arg_x2 >> 16);
    d1 = (uint16_t)(d1 - rd16(m, a5 + 0x0998u));
    if ((int16_t)d1 < 0) {
        d1 = 0u;
    } else {
        d1 = asr16((uint16_t)d1, 2);
        if ((uint16_t)d1 >= rd16(m, a5 + 0x099Cu)) {
            d1 = (uint16_t)(rd16(m, a5 + 0x099Cu) - 1u);
        }
    }
    a1 = rd32(m, a5 + 0x0988u);

    /* 0x0BD838 */
    d1 = rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d1 * 2u);
    d2 = rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d2 * 2u);
    d2 = (uint16_t)(d2 - d1);
    d0 = 0x7FFFu;
    d3 = arg_y;
    a0 = 0x000BD5AEu;
    d1 = (uint16_t)(-(int16_t)d1);
    d1 = (uint16_t)(d1 - 1u);
    d1 = (uint16_t)(d1 + d1);
    for (;;) {
        /* 0x0BD856 */
        a2 = a1 + (uint32_t)(int32_t)(int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d1);
        a2 -= 4u;
        do {
            a2 += 4u;
        } while ((int16_t)d3 > (int16_t)rd16(m, a2));
        if ((int16_t)d0 > (int16_t)rd16(m, a2)) {           /* 0x0BD866 ble */
            d0 = rd16(m, a2);
            a2 += 2u;
            a0 = a2 + (uint32_t)(int32_t)(int16_t)rd16(m, a2);
        }
        d1 = (uint16_t)(d1 - 2u);                          /* 0x0BD86E */
        if (d2 == 0u)
            break;
        d2--;
    }
    *out_w = (uint16_t)d0;
    *out_a0 = a0;
}

/* 0x0C555A: tile span of the object's sprite (D7/D6 = width, height * 16). */
static void h_C555A(const f3mem_t *m, uint32_t a4, uint32_t *d6p, uint32_t *d7p,
                    uint32_t *a0p)
{
    /* 0x0C555A movea.l ([$9c,a4]),a0: memory-indirect, so the pointer is the
     * long at (a4+0x9C) and a0 is the long stored there. */
    uint32_t a0 = rd32(m, rd32(m, a4 + 0x09Cu));
    uint32_t d7, d6;

    d7 = set_word(*d7p, rd16(m, a0));
    a0 += 2u;
    d6 = set_word(*d6p, rd16(m, a0));
    a0 += 2u;
    d7 = set_word(d7, (uint16_t)((uint16_t)d7 << 4));
    d6 = set_word(d6, (uint16_t)((uint16_t)d6 << 4));
    d7 = set_word(d7, (uint16_t)((uint16_t)d7 + 0x000Fu));
    d6 = set_word(d6, (uint16_t)((uint16_t)d6 + 0x000Fu));

    *d6p = d6; *d7p = d7; *a0p = a0;
}

/* 0x0C5572: find the free tile column in front of the object; D0.w = -1 when
 * there is no room, 0 otherwise. */
static uint16_t h_C5572(const f3mem_t *m, uint32_t a4, uint32_t a5, uint32_t *d0p,
                        uint32_t *d1p, uint32_t *d2p, uint32_t *d3p, uint32_t *d4p,
                        uint32_t *d5p, uint32_t *d6p, uint32_t *d7p, uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = 0u, d3 = 0u, d4 = 0u, d5 = 0u,
             d6 = *d6p, d7 = *d7p,
             a0 = *a0p;
    uint16_t out_w;
    uint32_t out_a;

    d2 = 4u;
    d3 = 8u;
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 + rd16(m, a4 + 0x0A0u)));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 + rd16(m, a4 + 0x0A4u)));
    d1 = set_word(d1, (uint16_t)d3);
    d0 = set_word(d0, (uint16_t)d2);
    (void)h_BD5D8(m, a5, &d0, &d1, &d2, &a0);
    if ((uint16_t)d0 != 0x8000u) {                          /* 0x0C558C bne */
        d0 = 0u;
        goto out;
    }
    d1 = set_word(d1, (uint16_t)d3);
    d0 = set_word(d0, (uint16_t)d2);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d6));
    (void)h_BD5D8(m, a5, &d0, &d1, &d2, &a0);
    if ((uint16_t)d0 == 0x8000u) {                          /* 0x0C55A0 beq */
        d0 = 0xFFFFFFFFu;
        goto out;
    }
    d4 = set_word(d4, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)((uint16_t)d4 + (uint16_t)d6));
    h_BD790(m, a5, (uint16_t)d3, ((uint16_t)d2 << 16) | (uint16_t)d4, (uint16_t)d4,
            &out_w, &out_a);
    d0 = set_word(d0, out_w);
    a0 = out_a;
    d5 = set_word(d5, (uint16_t)d3);
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 + (uint16_t)d7));
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
    if ((int16_t)d5 > (int16_t)d0)                          /* 0x0C55C0 ble */
        d5 = set_word(d5, (uint16_t)d0);
    for (;;) {
        /* 0x0C55CA */
        d1 = set_word(d1, (uint16_t)d3);
        d0 = set_word(d0, (uint16_t)d4);
        (void)h_BD5D8(m, a5, &d0, &d1, &d2, &a0);
        if ((uint16_t)d0 == 0x8000u)                        /* 0x0C55D8 bne */
            break;
        d4 = set_word(d4, rd16(m, a0));                     /* 0x0C55C6 */
        d4 = set_word(d4, (uint16_t)((uint16_t)d4 - 1u));
    }
    d4 = set_word(d4, (uint16_t)((uint16_t)d4 - 0x002Fu));
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 - 0x002Fu));
    d4 = set_word(d4, (uint16_t)((uint16_t)d4 - rd16(m, a5 + 0x1BC4u)));
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 - rd16(m, a5 + 0x1BC6u)));
    if (rd16(m, a5 + 0x1CFAu) != 0) {
        a0 = rd32(m, a5 + 0x1D14u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0x00C0u);
        a0 -= 2u; wr16(m, a0, (uint16_t)d5);
        a0 -= 2u; wr16(m, a0, (uint16_t)d4);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 4u; wr32(m, a0, 0x0003412Eu);
        wr16(m, a5 + 0x1CFAu, (uint16_t)(rd16(m, a5 + 0x1CFAu) - 1u));
        wr32(m, a5 + 0x1D14u, a0);
    }
    d0 = 0u;
out:
    *d0p = d0; *d1p = d1; *d2p = d2; *d3p = d3; *d4p = d4;
    *d5p = d5; *d6p = d6; *d7p = d7; *a0p = a0;
    return (uint16_t)d0;
}

/* 0x0C5622: walk the wall in front of the object and collect the tiles. */
static uint16_t h_C5622(const f3mem_t *m, uint32_t a4, uint32_t a5, uint32_t *d0p,
                        uint32_t *d1p, uint32_t *d2p, uint32_t *d3p, uint32_t *d4p,
                        uint32_t *d5p, uint32_t *d6p, uint32_t *d7p, uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = 0u, d3 = 0u, d4 = 0u, d5 = 0u,
             d6 = *d6p, d7 = *d7p,
             a0 = *a0p;
    uint16_t out_w;
    uint32_t out_a;

    d2 = set_word(4u, (uint16_t)(4u + rd16(m, a4 + 0x0A0u)));
    d3 = set_word(0u, rd16(m, a4 + 0x0A4u));
    d4 = set_word(0u, (uint16_t)d6);
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 + (uint16_t)d7));
    d5 = set_word(0u, (uint16_t)d3);
    d4 = set_word(d4, asr16((uint16_t)d4, 1));
    d1 = set_word(d1, (uint16_t)d3);
    d0 = set_word(d0, (uint16_t)d2);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d4));
    (void)h_BD546(m, a5, &d0, &d1, &a0);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
    if ((uint16_t)d0 == 0u)
        goto hit;                                           /* 0x0C56C0 */
    d1 = set_word(d1, (uint16_t)d3);
    d0 = set_word(d0, (uint16_t)d2);
    (void)h_BD546(m, a5, &d0, &d1, &a0);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
    if ((uint16_t)d0 == 0u)
        goto hit;
    d1 = set_word(d1, (uint16_t)d3);
    d0 = set_word(d0, (uint16_t)d2);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d6));
    (void)h_BD546(m, a5, &d0, &d1, &a0);
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - 1u));
    if ((uint16_t)d0 == 0u)
        goto hit;
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - 4u));
    d4 = set_word(d4, (uint16_t)d2);
    d4 = set_word(d4, (uint16_t)((uint16_t)d4 + (uint16_t)d6));
    h_BD790(m, a5, (uint16_t)d3, ((uint16_t)d2 << 16) | (uint16_t)d4, (uint16_t)d4,
            &out_w, &out_a);
    d0 = set_word(d0, out_w);
    a0 = out_a;
    d5 = set_word(d5, (uint16_t)((uint16_t)d5 - 1u));
    if ((int16_t)d5 < (int16_t)d0)                          /* 0x0C5684 blt */
        goto out;
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BC4u)));
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x1BC6u)));
    if (rd16(m, a5 + 0x1CFAu) != 0) {
        a0 = rd32(m, a5 + 0x1D14u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0x00C0u);
        a0 -= 2u; wr16(m, a0, (uint16_t)d0);
        a0 -= 2u; wr16(m, a0, (uint16_t)d2);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 4u; wr32(m, a0, 0x000340B2u);
        wr16(m, a5 + 0x1CFAu, (uint16_t)(rd16(m, a5 + 0x1CFAu) - 1u));
        wr32(m, a5 + 0x1D14u, a0);
    }
    goto out;

hit:
    /* 0x0C56C0 */
    d0 = set_word(d0, rd16(m, a0 + 0x000Cu));
    d1 = set_word(d1, rd16(m, a0 + 0x003Cu));
    d0 = set_word(d0, (uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x1BC4u)));
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x1BC6u)));
    if (rd16(m, a5 + 0x1CFAu) != 0) {
        a0 = rd32(m, a5 + 0x1D14u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0x00C0u);
        a0 -= 2u; wr16(m, a0, (uint16_t)d1);
        a0 -= 2u; wr16(m, a0, (uint16_t)d0);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 4u; wr32(m, a0, 0x000340A6u);
        wr16(m, a5 + 0x1CFAu, (uint16_t)(rd16(m, a5 + 0x1CFAu) - 1u));
        wr32(m, a5 + 0x1D14u, a0);
    }
out:
    *d0p = d0; *d1p = d1; *d2p = d2; *d3p = d3; *d4p = d4;
    *d5p = d5; *d6p = d6; *d7p = d7; *a0p = a0;
    return (uint16_t)d0;
}

/* 0x0C54D8: draw the "reveal" line of tiles for the closed door. */
static void h_C54D8(const f3mem_t *m, uint32_t a4, uint32_t a5, uint32_t *d0p,
                    uint32_t *d1p, uint32_t *d2p, uint32_t *d3p, uint32_t *d4p,
                    uint32_t *d5p, uint32_t *d6p, uint32_t *d7p, uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = *d2p, d3 = *d3p, d4 = *d4p;
    uint32_t d5 = *d5p, d6 = *d6p, d7 = *d7p, a0 = *a0p;

    d0 = set_word(d0, (uint16_t)(rd16(m, a5 + 0x0978u) & 1u));
    if ((int16_t)(uint16_t)d0 != (int16_t)rd16(m, a4 + 4u))
        goto out;
    if (rd16(m, a5 + 0x1BF0u) != 0u)
        goto out;
    if ((int16_t)rd16(m, a4 + 0x082Au) >= 0)
        goto out;
    if (rd16(m, a4 + 0x0096u) == 0u)
        goto out;

    h_C555A(m, a4, &d6, &d7, &a0);
    (void)h_C5572(m, a4, a5, &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, &a0);
    d0 = set_word(d0, (uint16_t)d0);
    if ((int16_t)(uint16_t)d0 < 0)                          /* 0x0C5500 bmi */
        goto out;
    (void)h_C5622(m, a4, a5, &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, &a0);

    /* 0x0C5508 move.w ([$9c,a4],$6),d1 */
    d1 = set_word(d1, rd16(m, rd32(m, a4 + 0x009Cu) + 6u));
    d0 = set_word(d0, rd16(m, a4 + 0x006Au));
    d1 = set_word(d1, (uint16_t)((uint16_t)d1 ^ (uint16_t)d0));
    d2 = set_word(4u, (uint16_t)(4u + rd16(m, a4 + 0x0A0u)));
    d3 = set_word(8u, (uint16_t)(8u + rd16(m, a4 + 0x0A4u)));
    d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BC4u)));
    d3 = set_word(d3, (uint16_t)((uint16_t)d3 - rd16(m, a5 + 0x1BC6u)));
    if (rd16(m, a5 + 0x1CFAu) != 0) {
        a0 = rd32(m, a5 + 0x1D14u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0x0048u);
        a0 -= 2u; wr16(m, a0, (uint16_t)d3);
        a0 -= 2u; wr16(m, a0, (uint16_t)d2);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 2u; wr16(m, a0, 0u);
        a0 -= 4u; wr32(m, a0, rd32(m, rd32(m, a4 + 0x009Cu)));  /* move.l ([$9c,a4]) */
        wr16(m, a5 + 0x1CFAu, (uint16_t)(rd16(m, a5 + 0x1CFAu) - 1u));
        wr32(m, a5 + 0x1D14u, a0);
    }
out:
    *d0p = d0; *d1p = d1; *d2p = d2; *d3p = d3; *d4p = d4;
    *d5p = d5; *d6p = d6; *d7p = d7; *a0p = a0;
}

/* 0x0BEE3C: the countdown value at +0x52, rotated into place and clamped to
 * the per-character bound table at 0x0BEE30. */
static uint16_t h_BEE3C(const f3mem_t *m, uint32_t a4, uint32_t *d0p, uint32_t *d1p)
{
    uint32_t d0 = *d0p, d1 = *d1p, idx;

    d0 = rd32(m, a4 + 0x0052u);
    d1 = set_word(d1, rd16(m, a4 + 0x004Eu));
    d0 = (d0 << 16) | (d0 >> 16);                  /* swap d0 */
    d0 = (d0 << 4) | (d0 >> 28);                   /* rol.l #4,d0 */
    idx = 0x000BEE30u + (uint32_t)(int32_t)(int16_t)d1 * 4u;
    if (!outside(m, idx, (uint16_t)d0)) {           /* 0x0BEE4E bcc */
        *d0p = d0; *d1p = d1;
        return (uint16_t)d0;
    }
    if ((int16_t)(uint16_t)d0 > (int16_t)rd16(m, idx)) {
        d0 = set_word(d0, rd16(m, idx));            /* 0x0BEE5A */
    } else {
        d0 = 0u;                                    /* 0x0BEE56 moveq #0 */
    }
    *d0p = d0; *d1p = d1;
    return (uint16_t)d0;
}

/* 0x0BEF1A: write the "TIME" digits of the row into GFX RAM 0x61C0xx. */
static void h_BEF1A(const f3mem_t *m, uint32_t a4, f3regs_t *r, uint32_t *d0p,
                    uint32_t *d1p)
{
    uint32_t d0 = *d0p, d1 = *d1p;

    d0 = set_word(d0, rd16(m, a4 + 0x0106u));
    if ((int16_t)d0 < 0)                            /* 0x0BEF1E bpl */
        d0 = 0u;
    d1 = set_word(d1, rd16(m, a4 + 0x0050u));
    if ((int16_t)d1 <= 0)                           /* 0x0BEF26 bgt */
        d1 = 0u;
    d1 &= 0x0000000Fu;
    if (rd16(m, a4 + 4u) != 0u) {                   /* 0x0BEF30 bne */
        wr32(m, 0x61C04Cu, 0x06320650u);
        d1 += 0x102A1030u;
        wr32(m, 0x61C0CCu, d1);
        *d0p = d0; *d1p = d1;
        if ((uint16_t)d0 == rd16(m, a4 + 0x0852u))  /* 0x0BEFB6 */
            return;
        wr16(m, a4 + 0x0852u, (uint16_t)d0);
        wr16(m, 0x61C14Au, 0x0442u);
        if ((int16_t)(uint16_t)d0 < 0x0A) {         /* 0x0BEFC8 blt */
            wr16(m, 0x61C14Cu, 0u);
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 & 0x000Fu));
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 + 0x1030u));
            wr16(m, 0x61C14Eu, (uint16_t)d0);
            *d0p = d0;
            return;
        }
        d0 &= 0x0000007Fu;
        call_6AA8(m, r, d0, 0x61C14Cu, 2u, 8u);
        *d0p = r->d[0];
        return;
    }
    wr32(m, 0x61C000u, 0x08310850u);
    d1 += 0x102A1030u;
    wr32(m, 0x61C080u, d1);
    *d0p = d0; *d1p = d1;
    if ((uint16_t)d0 == rd16(m, a4 + 0x0852u))      /* 0x0BEF4C */
        return;
    wr16(m, a4 + 0x0852u, (uint16_t)d0);
    wr16(m, 0x61C100u, 0x0442u);
    if ((int16_t)(uint16_t)d0 < 0x0A) {             /* 0x0BEF5E blt */
        wr16(m, 0x61C102u, 0u);
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 & 0x000Fu));
        d0 = set_word(d0, (uint16_t)((uint16_t)d0 + 0x1030u));
        wr16(m, 0x61C104u, (uint16_t)d0);
        *d0p = d0;
        return;
    }
    d0 &= 0x0000007Fu;
    call_6AA8(m, r, d0, 0x61C102u, 2u, 8u);
    *d0p = r->d[0];
}

/* 0x0BF03A: write the credit-count digits into GFX RAM 0x61C0xx. */
static void h_BF03A(const f3mem_t *m, uint32_t a4, f3regs_t *r, uint32_t *d0p,
                    uint32_t *d1p, uint32_t *d2p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = *d2p;

    d0 = set_word(d0, rd16(m, a4 + 0x00B2u));
    if ((uint16_t)d0 == 0u) {                       /* 0x0BF03E beq */
        wr16(m, a4 + 0x0854u, 0u);
        wr16(m, a4 + 0x0856u, 0u);
        if (rd16(m, a4 + 4u) != 0u) {
            wr16(m, 0x61C140u, 0u);
            wr32(m, 0x61C142u, 0u);
            wr32(m, 0x61C146u, 0u);
        } else {
            wr32(m, 0x61C106u, 0u);
            wr32(m, 0x61C10Au, 0u);
            wr16(m, 0x61C10Eu, 0u);
        }
        *d0p = d0; *d1p = d1; *d2p = d2;
        return;
    }
    if ((uint16_t)d0 == rd16(m, a4 + 0x0856u)) {    /* 0x0BF044 bne */
        if ((uint16_t)d0 == 0u)
            goto zero;
        d1 = set_word(d1, rd16(m, a4 + 0x00B4u));
        if ((uint16_t)d1 == rd16(m, a4 + 0x0854u))
            goto out;                               /* 0x0BF052 beq -> rts */
        goto store;
    }
    wr16(m, a4 + 0x0856u, (uint16_t)d0);
    d2 = set_word(d2, 0x044Du);
    if ((uint16_t)d0 != 1u)                         /* 0x0BF062 beq */
        d2 = set_word(d2, 0x044Cu);
    if (rd16(m, a4 + 4u) != 0u)
        wr16(m, 0x61C140u, (uint16_t)d2);
    else
        wr16(m, 0x61C108u, (uint16_t)d2);
    d1 = set_word(d1, rd16(m, a4 + 0x00B4u));
store:
    /* 0x0BF080 */
    wr16(m, a4 + 0x0854u, (uint16_t)d1);
    d1 &= 0x000003FFu;
    if (rd16(m, a4 + 4u) != 0u) {                   /* 0x0BF08E bne */
        if ((int16_t)(uint16_t)d1 < 0x64) {         /* 0x0BF0FA blt */
            if ((int16_t)(uint16_t)d1 < 0x0A) {     /* 0x0BF11C blt */
                wr16(m, 0x61C142u, 0u);
                wr16(m, 0x61C144u, 0u);
                d1 = set_word(d1, (uint16_t)((uint16_t)d1 & 0x000Fu));
                d1 = set_word(d1, (uint16_t)((uint16_t)d1 + 0x1030u));
                wr16(m, 0x61C146u, (uint16_t)d1);
                goto out;
            }
            wr16(m, 0x61C142u, 0u);
            call_6AA8(m, r, d1, 0x61C144u, 2u, 8u);
            goto out;
        }
        call_6AA8(m, r, d1, 0x61C142u, 3u, 8u);
        goto out;
    }
    if ((int16_t)(uint16_t)d1 < 0x64) {             /* 0x0BF094 blt */
        if ((int16_t)(uint16_t)d1 < 0x0A) {         /* 0x0BF0B6 blt */
            wr16(m, 0x61C10Au, 0u);
            wr16(m, 0x61C10Cu, 0u);
            d1 = set_word(d1, (uint16_t)((uint16_t)d1 & 0x000Fu));
            d1 = set_word(d1, (uint16_t)((uint16_t)d1 + 0x1030u));
            wr16(m, 0x61C10Eu, (uint16_t)d1);
            goto out;
        }
        wr16(m, 0x61C10Au, 0u);
        call_6AA8(m, r, d1, 0x61C10Cu, 2u, 8u);
        goto out;
    }
    call_6AA8(m, r, d1, 0x61C10Au, 3u, 8u);
    goto out;
zero:
    wr16(m, a4 + 0x0854u, 0u);
    wr16(m, a4 + 0x0856u, 0u);
    if (rd16(m, a4 + 4u) != 0u) {
        wr16(m, 0x61C140u, 0u);
        wr32(m, 0x61C142u, 0u);
        wr32(m, 0x61C146u, 0u);
    } else {
        wr32(m, 0x61C106u, 0u);
        wr32(m, 0x61C10Au, 0u);
        wr16(m, 0x61C10Eu, 0u);
    }
out:
    *d0p = d0; *d1p = d1; *d2p = d2;
}

/* 0x0BEE60: sprite row + countdown digits + credit digits + the aged timer. */
static void h_BEE60(const f3mem_t *m, uint32_t a4, f3regs_t *r,
                    uint32_t *d0p, uint32_t *d1p, uint32_t *d2p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = *d2p, a0, a1;

    (void)h_BEE3C(m, a4, &d0, &d1);
    if ((uint16_t)d0 == rd16(m, a4 + 0x0850u))              /* 0x0BEE66 beq */
        goto after;
    wr16(m, a4 + 0x0850u, (uint16_t)d0);
    d1 = set_word(d1, rd16(m, a4 + 0x004Eu));
    d2 = 7u;
    if (rd16(m, a4 + 4u) == 0u) {                           /* 0x0BEE76 bne */
        a0 = rd32(m, 0x000D1114u + (uint32_t)(int32_t)(int16_t)d1 * 4u);
        d0 = set_word(d0, rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d0 * 2u));
        a0 += (uint32_t)(int32_t)(int16_t)d0;
        a1 = 0x0061C084u;
    } else {
        a0 = rd32(m, 0x000D1120u + (uint32_t)(int32_t)(int16_t)d1 * 4u);
        d0 = set_word(d0, rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d0 * 2u));
        a0 += (uint32_t)(int32_t)(int16_t)d0;
        a1 = 0x0061C0BCu;
    }
    for (;;) {                                              /* 0x0BEEA8 */
        wr16(m, a1, rd16(m, a0));
        a0 += 2u;
        a1 += 2u;
        if (d2 == 0u)
            break;
        d2--;
    }
after:
    h_BEF1A(m, a4, r, &d0, &d1);
    h_BF03A(m, a4, r, &d0, &d1, &d2);
    /* 0x0BEEB6 */
    d0 = rd32(m, a4 + 0x005Eu);
    if ((int32_t)d0 < 0) {
        d0 = 0u;
    } else if (d0 > 0x000F423Fu) {
        d0 = 0x000F423Fu;
        wr32(m, a4 + 0x005Eu, d0);
    }
    if (rd32(m, a4 + 0x084Cu) == d0)                        /* 0x0BEED6 beq */
        goto out;
    wr32(m, a4 + 0x084Cu, d0);
    if (rd16(m, a4 + 4u) != 0u) {
        call_6AA8(m, r, d0, 0x0061C040u, 6u, 8u);
        d0 = r->d[0];
        goto out;
    }
    call_6AA8(m, r, d0, 0x0061C004u, 6u, 8u);
    d0 = r->d[0];
out:
    *d0p = d0; *d1p = d1; *d2p = d2;
}

int Ea_0BEB9A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t *D = r->d, *A = r->a;
    uint32_t a5 = A[5], a4 = A[4];
    uint32_t sp0 = A[7];

    /* 0x0BEB9A move.w $8da(a4),d0 ; ror.w #8,d0 ; move.w d0,$8da(a4) ; bmi */
    {
        uint16_t w = rd16(m, a4 + 0x08DAu);
        uint16_t rot = (uint16_t)((w >> 8) | (w << 8));

        D[0] = set_word(D[0], rot);
        wr16(m, a4 + 0x08DAu, rot);
        if (rot & 0x8000u)
            goto blink_on;
    }
    /* 0x0BEBA6 tst.w $2(a4) ; blt.w $bed56 */
    if ((int16_t)rd16(m, a4 + 2u) >= 0) {
        A[7] = sp0 + 4u;                                   /* 0x0BEBAE rts */
        return 0;
    }
    goto cont;

blink_on:
    /* 0x0BEBB0 */
    {
        uint32_t d5 = 0x08u, d6 = 0x60u, a2;

        if (rd16(m, a4 + 4u) != 0u) {                      /* 0x0BEBB8 beq */
            d6 = 0x70u;
            d5 = 0x118u;
        }
        D[0] = set_word(D[0], rd16(m, a4 + 0x004Eu));
        a2 = 0x000BEC8Au + (uint32_t)(int32_t)(int16_t)D[0] * 8u;
        d6 |= rd32(m, a2);                                 /* 0x0BEBC8 or.l (a2)+,d6 */
        a2 = rd32(m, a2 + 4u);                             /* 0x0BEBCC movea.l (a2)+,a2 */
        if (rd16(m, a4 + 4u) != 0u) {
            wr16(m, a5 + 0x1C94u, (uint16_t)d6);
            wr32(m, a5 + 0x1C96u, a2);
        } else {
            wr16(m, a5 + 0x1C8Au, (uint16_t)d6);
            wr32(m, a5 + 0x1C8Cu, a2);
        }
        D[5] = d5;
        D[6] = d6;
    }
    /* 0x0BEBE8 tst.b $8db(a4) ; beq.w $bec84 */
    if (rd8(m, a4 + 0x08DBu) == 0u) {
        wr16(m, a4 + 0x08DAu, 0u);
        A[7] = sp0 + 4u;
        return 0;
    }
    {
        uint32_t a0 = 0x000BED4Au;
        uint32_t a2 = 0x000BECA2u;
        uint32_t d0 = D[0], d1 = D[1], d2 = D[2], d3 = D[3];

        /* 0x0BEBF4 */
        d0 = set_word(D[0], rd16(m, a5 - 0x6104u));
        d1 = set_word(D[1], rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d0 * 2u));
        if (rd16(m, a5 + 0x1BEEu) == 0u) {                  /* 0x0BEC04 bne */
            a2 = 0x000BECC6u;
            d0 = set_word(D[0], rd16(m, a4 + 0x004Cu));
            if ((int16_t)d0 < 0) {                         /* 0x0BEC0E bpl */
                d0 = 0u;
            } else {
                d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;   /* ext.l */
                d0 = divu16(d0, 0x3Cu);                    /* 0x0BEC14 */
                if ((int16_t)(uint16_t)d0 > 9)             /* 0x0BEC1C ble */
                    d0 = 9u;
            }
            a0 = 0x000BECFAu + (uint32_t)(int32_t)(int16_t)d0 * 8u;
            d0 = rd32(m, a0);
            a0 += 4u;
            d2 = set_word(D[2], rd16(m, a0));
            a0 += 2u;
            d3 = set_word(D[3], rd16(m, a0));
            d2 = set_word(d2, (uint16_t)((uint16_t)d2 + (uint16_t)D[5]));
            emit_entry(m, a5, d0, (uint16_t)d2, (uint16_t)d3, (uint16_t)d1);
        }
        D[0] = d0; D[1] = d1; D[2] = d2; D[3] = d3;
        /* 0x0BEC7E: the a2 list, 8-byte entries, positive tag keeps it going */
        for (;;) {
            if ((int32_t)rd32(m, a2) <= 0)
                break;
            d0 = rd32(m, a2);
            a2 += 4u;
            d2 = set_word(D[2], rd16(m, a2));
            a2 += 2u;
            d3 = set_word(D[3], rd16(m, a2));
            a2 += 2u;
            d2 = set_word(d2, (uint16_t)((uint16_t)d2 + (uint16_t)D[5]));
            emit_entry(m, a5, d0, (uint16_t)d2, (uint16_t)d3, (uint16_t)d1);
            D[0] = d0; D[2] = d2; D[3] = d3;
        }
    }
    A[7] = sp0 + 4u;
    return 0;

cont:
    /* 0x0BED56: mark the line byte of this panel */
    {
        A[0] = a5 + 0x1C8Au;
        if (rd16(m, a4 + 4u) != 0u)                        /* 0x0BED5E beq */
            A[0] = a5 + 0x1C94u;
        wr8(m, A[0], 0xFFu);                               /* 0x0BED64 st.b */
    }
    /* 0x0BED66 tst.l $56(a4) ; ble.w $bee2e */
    if ((int32_t)rd32(m, a4 + 0x0056u) <= 0) {
        A[7] = sp0 + 4u;
        return 0;
    }
    /* 0x0BED6E */
    if ((int16_t)rd16(m, a4 + 0x082Au) >= 0) {             /* 0x0BED72 bmi */
        wr16(m, a4 + 0x082Au, (uint16_t)(rd16(m, a4 + 0x082Au) - 1u));
        D[0] = set_word(D[0], (uint16_t)(rd16(m, a5 + 0x0978u) & 2u));
        D[0] = set_word(D[0], (uint16_t)((uint16_t)D[0] >> 1));
        if ((int16_t)(uint16_t)D[0] != (int16_t)rd16(m, a4 + 4u))
            goto after_c54d8;                              /* 0x0BED86 bne $bedfa */
    }
    /* 0x0BED8A */
    if (rd16(m, a4 + 0x0096u) == 0u)
        goto after_c54d8;                                  /* 0x0BED8E beq $bedfa */
    if ((int32_t)rd32(m, a4 + 0x009Cu) <= 0)               /* 0x0BED96 ble */
        goto after_c54d8;
    {
        uint32_t a0, a1, d1, d2, d3, d4;

        A[1] = rd32(m, a4 + 0x009Cu);                      /* 0x0BED9A movea.l */
        a1 = A[1];
        d1 = set_word(D[1], rd16(m, a1 + 6u));
        D[0] = set_word(D[0], rd16(m, a4 + 0x006Au));
        d1 = set_word(d1, (uint16_t)((uint16_t)d1 ^ (uint16_t)D[0]));
        D[1] = d1;
        d2 = set_word(D[2], rd16(m, a4 + 0x00A0u));
        d3 = set_word(D[3], rd16(m, a4 + 0x00A4u));
        d2 = set_word(d2, (uint16_t)((uint16_t)d2 - rd16(m, a5 + 0x1BC4u)));
        d3 = set_word(d3, (uint16_t)((uint16_t)d3 - rd16(m, a5 + 0x1BC6u)));
        d4 = set_word(D[4], (uint16_t)(rd16(m, a4 + 0x0098u) & 7u));
        D[2] = d2; D[3] = d3; D[4] = d4;
        a0 = a5 + 0x1CF0u + (uint32_t)d4 * 2u;             /* 0x0BEDBE */
        if (rd16(m, a0) == 0u) {                           /* 0x0BEDC8 beq $bedf4 */
            A[0] = a0;                                     /* A0 = the counter cell */
        } else {
            wr16(m, a0, (uint16_t)(rd16(m, a0) - 1u));
            a0 = a5 + 0x1D00u + (uint32_t)d4 * 4u;
            D[4] = a0;                                     /* 0x0BEDD4 move.l a0,d4 */
            a0 = rd32(m, a0);
            a0 -= 2u; wr16(m, a0, 0u);
            a0 -= 2u; wr16(m, a0, (uint16_t)d1);
            a0 -= 2u; wr16(m, a0, rd16(m, a4 + 0x009Au));
            a0 -= 2u; wr16(m, a0, (uint16_t)d3);
            a0 -= 2u; wr16(m, a0, (uint16_t)d2);
            a0 -= 2u; wr16(m, a0, 0u);
            a0 -= 2u; wr16(m, a0, 0u);
            a0 -= 4u; wr32(m, a0, rd32(m, a1));            /* 0x0BEDEE move.l (a1) */
            wr32(m, D[4], a0);                             /* 0x0BEDF0 move.l a0,(d4) */
            A[0] = a0;
        }
    }
    /* 0x0BEDF4 jsr $c54d8 */
    {
        uint32_t d0 = D[0], d1 = D[1], d2 = D[2], d3 = D[3];
        uint32_t d4 = D[4], d5 = D[5], d6 = D[6], d7 = D[7];
        uint32_t a0 = A[0];

        h_C54D8(m, a4, a5, &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, &a0);
        D[0] = d0; D[1] = d1; D[2] = d2; D[3] = d3;
        D[4] = d4; D[5] = d5; D[6] = d6; D[7] = d7;
        A[0] = a0;
    }
after_c54d8:
    /* 0x0BEDFA add.l $62(a4),$5e(a4) */
    D[0] = rd32(m, a4 + 0x0062u);
    if ((int32_t)D[0] < 0)                                 /* 0x0BEDFE bpl */
        D[0] = 0u;
    wr32(m, a4 + 0x005Eu, rd32(m, a4 + 0x005Eu) + D[0]);
    if ((int32_t)rd32(m, a4 + 0x005Eu) > 0x000F423F) {     /* 0x0BEE0E ble */
        D[0] = rd32(m, a4 + 0x005Eu);
        D[0] = divu16(D[0], 0x64u);                        /* 0x0BEE14 divu.w #$64 */
        D[0] = (D[0] << 16) | (D[0] >> 16);                /* swap d0 */
        D[0] = (uint32_t)(int32_t)(int16_t)(uint16_t)D[0]; /* ext.l d0 */
        D[0] += 0x000F41DCu;
        wr32(m, a4 + 0x005Eu, D[0]);
    }
    /* 0x0BEE26 bsr $bee60 */
    h_BEE60(m, a4, r, &D[0], &D[1], &D[2]);
    wr32(m, a4 + 0x0062u, 0u);                             /* 0x0BEE2A */
    A[7] = sp0 + 4u;                                       /* 0x0BEE2E rts */
    return 0;
}
