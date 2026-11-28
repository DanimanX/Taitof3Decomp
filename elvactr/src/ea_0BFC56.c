/* ROM 0x0BFC56: run the elevator-door/step state chain of the object at A4.
 * Walks the 10-byte state records hung off A4+0x72 until one of the handler
 * routines at 0x0BFED0..0x0C2922 reports "commit", switches A4+0x72/0x76 to
 * the chosen record, resolves the room/sprite cell at A4+0x9C from the
 * A4+0xB2 index, moves the two cell corners A4+0xA0/0xA4 by the delta of the
 * old and new cells (probes 0x0BD9F2), then refreshes the corner boxes
 * (0x0BFA96), the eased target box (0x0C019E) and the collision box
 * (0x0C04A2). */
#include "ea_auto.h"

/* cmp2.w (ea),Dn: true when v is outside the signed word bounds {lo,hi}. */
static int outside(const f3mem_t *m, uint32_t ea, uint16_t v)
{
    int16_t lo = (int16_t)rd16(m, ea), hi = (int16_t)rd16(m, ea + 2u);
    return (int16_t)v < lo || (int16_t)v > hi;
}

/* asr.w: arithmetic right shift of a 16-bit word. */
static uint16_t asr16(uint16_t v, unsigned n)
{
    int16_t s = (int16_t)v;
    return s >= 0 ? (uint16_t)(s >> n) : (uint16_t)~((uint16_t)(~s) >> n);
}

/* 0x0BD516: solidity of the map cell at (D0,D1). The map at A5+0x994 is a
 * row-major bitmap: the row index is (D1-A5+0x99A)>>1 and the column
 * (D0-A5+0x998)>>2 selects bit (col & 7) of byte (row + col>>3), counted from
 * the MSB. D0.w = 0xFFFF (solid, Z=0) or 0 (free, Z=1). */
static int h_BD516(const f3mem_t *m, uint32_t a5, uint32_t *d0p, uint32_t *d1p,
                   uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, a0 = *a0p;
    uint16_t col, row;

    col = asr16((uint16_t)((uint16_t)d0 - rd16(m, a5 + 0x0998u)), 2);
    d0 = set_word(d0, col);
    if (col >= rd16(m, a5 + 0x099Cu)) {                    /* 0x0BD520 bcc */
        d0 = 0u;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return 1;                                          /* 0x0BD542 moveq #0 */
    }
    row = asr16((uint16_t)((uint16_t)d1 - rd16(m, a5 + 0x099Au)), 1);
    d1 = set_word(d1, row);
    if (row >= rd16(m, a5 + 0x099Eu)) {                    /* 0x0BD52C bcc */
        d0 = 0u;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return 1;
    }
    d0 = (uint32_t)(int32_t)(int16_t)col;                  /* 0x0BD52E ext.l d0 */
    a0 = rd32(m, a5 + 0x0994u);                            /* 0x0BD530 */
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0 + (uint32_t)(int32_t)(int16_t)row * 2u);
    a0 += (uint32_t)((uint16_t)d0 >> 3);                   /* row byte holding the bit */
    /* 0x0BD538 bftst (a0){col:1} ; 0x0BD53C sne.b d0 ; ext.w d0 */
    d0 = set_word(d0, (rd8(m, a0) & (uint8_t)(0x80u >> ((uint16_t)d0 & 7u)))
                          ? 0xFFFFu : 0x0000u);

    *d0p = d0; *d1p = d1; *a0p = a0;
    return (uint16_t)d0 == 0u;                             /* 0x0BD540 rts, Z */
}

/* 0x0BD546: solidity probe at (D0,D1). D0/D1 are restored from the stack by
 * the routine's movem, so the ANSWER IS THE Z FLAG: Z=1 when the map cell and
 * every object box are clear, Z=0 on a hit (D0 then holds 1/2 or a stale
 * value, A0 the object). */
static int h_BD546(const f3mem_t *m, uint32_t a5, uint32_t *d0p, uint32_t *d1p,
                   uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, a0 = *a0p;
    uint16_t sd0 = (uint16_t)d0, sd1 = (uint16_t)d1;
    uint32_t d2, a1;
    int z;

    z = h_BD516(m, a5, &d0, &d1, &a0);
    d0 = (d0 & 0xFFFF0000u) | sd0;                     /* 0x0BD54C movem.w (a7)+,d0-d1 */
    d1 = (d1 & 0xFFFF0000u) | sd1;                     /* (the pops keep the flags) */
    if (!z) {                                          /* 0x0BD550 bne $bd59c */
        *d0p = d0; *d1p = d1; *a0p = a0;
        return 0;                                      /* Z=0: hit */
    }
    if ((int16_t)rd16(m, 0x41B818u) < 0) {             /* 0x0BD558 bmi $bd59a */
        d0 = 0u;
        *d0p = d0; *d1p = d1; *a0p = a0;
        return 1;                                      /* moveq #$0 -> Z=1 */
    }
    d2 = rd16(m, 0x41B818u);
    a1 = 0x41B81Cu;
    for (;;) {
        a0 = rd32(m, a1);
        a1 += 4u;
        if (!outside(m, a0 + 0x34u, (uint16_t)d0)) {           /* 0x0BDABA cmp2 */
            if (!outside(m, a0 + 0x3Cu, (uint16_t)d1)) {       /* 0x0BDAC0 */
                d0 = 1u;
                break;
            }
        }
        if (rd32(m, a0 + 0x24u) != 0u) {                       /* 0x0BDA98 */
            if (!outside(m, a0 + 0x24u, (uint16_t)d0)) {       /* 0x0BDA9E */
                if (!outside(m, a0 + 0x2Cu, (uint16_t)d1)) {   /* 0x0BDAA4 */
                    d0 = 2u;
                    break;
                }
            }
        }
        if (d2 == 0u) {                                /* 0x0BDAD6 dbra d3 */
            d0 = 0u;
            *d0p = d0; *d1p = d1; *a0p = a0;
            return 1;                                  /* Z=1: nothing there */
        }
        d2--;
    }
    *d0p = d0; *d1p = d1; *a0p = a0;
    return 0;                                          /* moveq #$1/#$2 -> NZ */
}

/* 0x0BFA46: read the packed signed-byte cell offset of side `side` from the
 * table at `tbl` (+0xA + side*2, high byte = x, low byte = y) and mirror x
 * inside the sprite cell when the attribute word is non-zero. */
static void h_BFA46(const f3mem_t *m, uint32_t tbl, uint16_t side, uint16_t attr,
                    uint32_t *xp, uint32_t *yp)
{
    uint32_t d0 = 0u, d1 = 0u, d2 = side;

    if ((int16_t)side >= 0) {
        if ((uint16_t)side <= 3u && attr != 0u)   /* 0x0BFA58 cmpi #3 ; bhi */
            d2 = (uint16_t)(side ^ 1u);           /* 0x0BFA64 eori.w #1,d2 */
        /* 0x0BFA68 move.w $a(a0,d2.w*2),d1 (extension 0x220A: scale 2) */
        d1 = rd16(m, tbl + 0x000Au + (uint32_t)(int16_t)d2 * 2u);
        /* 0x0BFA6C bfexts d1{16:8},d0 ; 0x0BFA70 ext.w d1 */
        d0 = (uint32_t)(int32_t)(int16_t)(int8_t)((d1 >> 8) & 0xFFu);
        d1 = (uint32_t)(int32_t)(int16_t)(int8_t)(d1 & 0xFFu);
    }
    if (attr != 0u) {                             /* 0x0BFA72 tst.w $1a(a7) */
        uint32_t t;

        d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)(0u - (uint16_t)d0);  /* neg.w */
        t = rd16(m, rd32(m, tbl) + 2u);           /* 0x0BFA7A move.w ([a0],$2),d2 */
        t = (uint16_t)((t + 1u) << 4);
        t = (uint16_t)(t - 1u);
        d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)d0 + (uint16_t)t);
    }
    *xp = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;
    *yp = (uint32_t)(int32_t)(int16_t)(uint16_t)d1;
}

/* 0x0BFA96: refresh the five cell corner pairs A4+0xD2..0xE4 from A4+0x9C. */
static void h_BFA96(const f3mem_t *m, uint32_t a4)
{
    uint32_t tbl, attr;
    uint32_t side, x, y;

    if (rd16(m, a4 + 0x080u) == 0u)               /* 0x0BFA96 tst.w $80(a4) */
        return;
    tbl = rd32(m, a4 + 0x09Cu);
    attr = rd16(m, a4 + 0x06Au);
    for (side = 0u; side < 5u; ++side) {          /* 0x0BFAA6..0x0BFB18 */
        h_BFA46(m, tbl, (uint16_t)side, (uint16_t)attr, &x, &y);
        wr16(m, a4 + 0x0D2u + side * 4u, (uint16_t)x);
        wr16(m, a4 + 0x0D4u + side * 4u, (uint16_t)y);
    }
}

/* 0x0BD9F2: map/object probe at (D0,D1); A0 = the hit object or the map row
 * entry, D0 = its top word, D1 = the probe y. */
static void h_BD9F2(const f3mem_t *m, uint32_t a5, uint32_t *d0p, uint32_t *d1p,
                    uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, a0 = *a0p;
    uint32_t d2 = (uint16_t)d0, d3 = 0xFFu, d4, a1, a2;
    uint16_t row;

    d4 = (uint16_t)d1;
    d4 = asr16((uint16_t)((uint16_t)d4 - rd16(m, a5 + 0x099Au)), 1);
    if (d4 >= rd16(m, a5 + 0x099Eu))                  /* 0x0BDA06 bcc $bd9bc */
        goto oob;
    d4 = (uint16_t)d0;
    d4 = asr16((uint16_t)((uint16_t)d4 - rd16(m, a5 + 0x0998u)), 2);
    if (d4 >= rd16(m, a5 + 0x099Cu))                  /* 0x0BDA14 bcc */
        goto oob;
    a0 = rd32(m, a5 + 0x0988u);                       /* 0x0BDA16 */
    row = rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d4 * 2u);
    d4 = (uint32_t)(int32_t)(int16_t)(uint16_t)(0u - row);   /* neg.w d4 */
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0 + (uint32_t)(int32_t)(int16_t)d4 * 2u - 2u);
    a0 -= 4u;
    do {                                              /* 0x0BDA26 */
        a0 += 4u;
    } while ((int16_t)d1 > (int16_t)rd16(m, a0));
    d0 = (d0 & 0xFFFF0000u) | rd16(m, a0);            /* 0x0BDA2C move.w (a0)+,d0 */
    a0 += 2u;
    a0 += (uint32_t)(int32_t)(int16_t)rd16(m, a0);    /* 0x0BDA2E adda.w (a0),a0 */
    a2 = 0x41B81Cu;                                   /* 0x0BDA30 */
    if (d3 == 0u)
        goto done;
    d3 = rd16(m, 0x41B818u);                          /* 0x0BDA8E */
    if ((int16_t)d3 < 0)
        goto done;
    for (;;) {                                        /* 0x0BDA96 */
        a1 = rd32(m, a2);
        a2 += 4u;
        if (rd32(m, a1 + 0x24u) != 0u) {              /* 0x0BDA98 tst.l $24(a1) */
            if (!outside(m, a1 + 0x24u, (uint16_t)d2)) {      /* 0x0BDA9E cmp2 */
                if ((int16_t)d0 > (int16_t)rd16(m, a1 + 0x2Cu)) { /* 0x0BDAA6 ble */
                    if ((int16_t)d1 <= (int16_t)rd16(m, a1 + 0x2Cu)) { /* 0x0BDAB0 bgt */
                        d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 0x2Cu);
                        a0 = a1 + 0x24u;
                    }
                }
            }
        }
        if (!outside(m, a1 + 0x34u, (uint16_t)d2)) {          /* 0x0BDABA cmp2 */
            if ((int16_t)d0 > (int16_t)rd16(m, a1 + 0x3Cu)) { /* 0x0BDAC2 ble */
                if ((int16_t)d1 <= (int16_t)rd16(m, a1 + 0x3Cu)) { /* 0x0BDACC bgt */
                    d0 = (d0 & 0xFFFF0000u) | rd16(m, a1 + 0x3Cu);
                    a0 = a1 + 0x34u;
                }
            }
        }
        if (d3 == 0u)                                 /* 0x0BDAD6 dbra d3 */
            break;
        d3--;
    }
done:
    *d0p = d0; *d1p = d1; *a0p = a0;
    return;
oob:
    d0 = (d0 & 0xFFFF0000u) | 0x8000u;                /* 0x0BD9BC move.w #$8000,d0 */
    a0 = 0x000BD5AEu;
    *d0p = d0; *d1p = d1; *a0p = a0;
}

/* 0x0C019E body: reload/ease the A4+0xE6..0x104 target box. */
static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

static uint32_t pack_sb(uint8_t b_hi, uint8_t b_lo)
{
    int16_t hi = (int16_t)(int8_t)b_hi, lo = (int16_t)(int8_t)b_lo;

    return ((uint32_t)(uint16_t)hi << 16) | (uint32_t)(uint16_t)lo;
}

static uint32_t wneg(uint32_t d)
{
    return (d & 0xFFFF0000u) | (uint16_t)(0u - (d & 0xFFFFu));
}

static uint32_t wadd(uint32_t d, uint32_t v)
{
    return (d & 0xFFFF0000u) | (uint16_t)((d & 0xFFFFu) + (v & 0xFFFFu));
}

/* 0x0C023A..0x0C03C6: one half-distance step (rounds away from zero). */
static uint32_t h_C019E_ease(const f3mem_t *m, uint32_t a4, uint32_t d0,
                             uint32_t src, uint32_t dst, uint16_t bias)
{
    uint16_t v = (uint16_t)(rd16(m, a4 + src) + bias);

    v = (uint16_t)(v - rd16(m, a4 + dst));
    if (v == 0u)
        return set_word(d0, v);
    if ((int16_t)v < 0) {
        v = asr16(v, 1);
    } else {
        v = (uint16_t)(0u - v);
        v = asr16(v, 1);
        v = (uint16_t)(0u - v);
    }
    wr16(m, a4 + dst, (uint16_t)(rd16(m, a4 + dst) + v));
    return set_word(d0, v);
}

static void h_C019E(const f3mem_t *m, uint32_t a4, uint32_t *d0p, uint32_t *d1p,
                    uint32_t *d2p, uint32_t *d3p, uint32_t *d4p, uint32_t *a0p)
{
    uint32_t d0 = *d0p, d1 = *d1p, d2 = *d2p, d3 = *d3p, d4 = *d4p, a0 = *a0p;

    if (rd16(m, a4 + 0x080u) != 0u) {                 /* 0x0C019E tst.w $80(a4) */
        a0 = rd32(m, a4 + 0x09Cu) + 0x18u;            /* 0x0C01A6 lea.l ([$9c,a4],$18) */
        d0 = pack_sb(rd8(m, a0), rd8(m, a0 + 1u));
        d1 = pack_sb(rd8(m, a0 + 2u), rd8(m, a0 + 3u));
        d2 = pack_sb(rd8(m, a0 + 4u), rd8(m, a0 + 5u));
        d3 = pack_sb(rd8(m, a0 + 6u), rd8(m, a0 + 7u));
        a0 += 8u;
        if (rd16(m, a4 + 0x06Au) != 0u) {             /* 0x0C01D6 */
            /* 0x0C01DC movea.l ([$9c,a4]),a0 - memory indirect, double load */
            uint32_t p = rd32(m, rd32(m, a4 + 0x09Cu));

            a0 = p;
            d4 = set_word(d4, (uint16_t)(rd16(m, p + 2u) + 1u));
            d4 = set_word(d4, (uint16_t)((uint16_t)d4 << 4));
            d4 = set_word(d4, (uint16_t)((uint16_t)d4 - 1u));
            d0 = wadd(wneg(d0), d4);
            d2 = wadd(wneg(d2), d4);
            d0 = swap32(d0);
            d2 = swap32(d2);
            d0 = wadd(wneg(d0), d4);
            d2 = wadd(wneg(d2), d4);
        }
        wr32(m, a4 + 0x0E6u, d0);
        wr32(m, a4 + 0x0EAu, d1);
        wr32(m, a4 + 0x0EEu, d2);
        wr32(m, a4 + 0x0F2u, d3);
        {
            uint16_t e2 = rd16(m, a4 + 0x0E2u), e4 = rd16(m, a4 + 0x0E4u);

            d0 = swap32((d0 & 0xFFFF0000u) | (uint16_t)(e2 - 3u));
            d0 = (d0 & 0xFFFF0000u) | (uint16_t)(e2 + 3u);
            d1 = swap32((d1 & 0xFFFF0000u) | (uint16_t)(e4 - 6u));
            d1 = (d1 & 0xFFFF0000u) | e4;
        }
        wr32(m, a4 + 0x0F6u, d0);
        wr32(m, a4 + 0x0FAu, d1);
        wr32(m, a4 + 0x0FEu, d0);
        wr32(m, a4 + 0x102u, d1);
    }
    d0 = h_C019E_ease(m, a4, d0, 0x0E6u, 0x0F6u, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0E8u, 0x0F8u, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0EAu, 0x0FAu, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0ECu, 0x0FCu, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0EEu, 0x0FEu, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0F0u, 0x100u, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0F2u, 0x102u, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0F4u, 0x104u, 0u);
    d0 = h_C019E_ease(m, a4, d0, 0x0E6u, 0x83Eu, rd16(m, a4 + 0x0A0u));
    d0 = h_C019E_ease(m, a4, d0, 0x0E8u, 0x840u, rd16(m, a4 + 0x0A0u));
    d0 = h_C019E_ease(m, a4, d0, 0x0EEu, 0x842u, rd16(m, a4 + 0x0A0u));
    d0 = h_C019E_ease(m, a4, d0, 0x0F0u, 0x844u, rd16(m, a4 + 0x0A0u));

    *d0p = d0; *d1p = d1; *d2p = d2; *d3p = d3; *d4p = d4; *a0p = a0;
}

/* 0x0C04A2: refresh the collision box A4+0x8E..0x94; D4 = the side value. */
static void h_C04A2(const f3mem_t *m, uint32_t a4, uint32_t *d4p)
{
    uint32_t d0, d1, d2, d3, d4 = *d4p, a0, x, y;

    /* 0x0C04A2 tst.w ([$72,a4],$c) */
    if (rd16(m, rd32(m, a4 + 0x072u) + 0x0Cu) != 0u) {
        a0 = rd32(m, a4 + 0x09Cu) + 0x1Cu;                 /* 0x0C04AC */
        d0 = (uint32_t)(int32_t)(int16_t)(int8_t)rd8(m, a0);
        d1 = (uint32_t)(int32_t)(int16_t)(int8_t)rd8(m, a0 + 1u);
        d2 = (uint32_t)(int32_t)(int16_t)(int8_t)(rd16(m, a0 + 2u) & 0xFFu); /* 0x0C04BE ext.w */
        d0 = set_word(d0, (uint16_t)d0 + (uint16_t)d1);     /* 0x0C04C0 */
        d0 = set_word(d0, asr16((uint16_t)d0, 1));          /* 0x0C04C2 */
        if (rd16(m, a4 + 0x06Au) != 0u) {                   /* 0x0C04C4 */
            a0 = rd32(m, rd32(m, a4 + 0x09Cu));             /* 0x0C04CA double load */
            d1 = set_word(d1, (uint16_t)(rd16(m, a0 + 2u) + 1u));
            d1 = set_word(d1, (uint16_t)((uint16_t)d1 << 4));
            d1 = set_word(d1, (uint16_t)((uint16_t)d1 - 1u));
            d0 = set_word(d0, (uint16_t)(0u - (uint16_t)d0));
            d0 = set_word(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d1));
        }
        wr16(m, a4 + 0x08Eu, (uint16_t)d0);
        wr16(m, a4 + 0x090u, (uint16_t)d2);
        wr16(m, a4 + 0x092u, (uint16_t)d0);
        wr16(m, a4 + 0x094u, (uint16_t)d2);
        *d4p = d4;
        return;
    }
    d3 = rd16(m, rd32(m, a4 + 0x09Cu) + 8u);               /* 0x0C04F0 */
    if ((uint16_t)d3 <= 3u) {                              /* 0x0C04F8 bhi */
        if (rd16(m, a4 + 0x06Au) != 0u)
            d3 = (uint16_t)(d3 ^ 1u);
    }
    h_BFA46(m, rd32(m, a4 + 0x09Cu), (uint16_t)d3, rd16(m, a4 + 0x06Au), &x, &y);
    d0 = (uint16_t)x;
    d1 = (uint16_t)y;
    d2 = rd16(m, a4 + 0x0E6u);
    if ((int16_t)d2 > (int16_t)rd16(m, a4 + 0x0EEu))
        d2 = rd16(m, a4 + 0x0EEu);
    if ((int16_t)d0 < (int16_t)d2)
        d0 = d2;
    d2 = rd16(m, a4 + 0x0E8u);
    if ((int16_t)d2 < (int16_t)rd16(m, a4 + 0x0F0u))
        d2 = rd16(m, a4 + 0x0F0u);
    if ((int16_t)d0 > (int16_t)d2)
        d0 = d2;
    wr16(m, a4 + 0x08Eu, (uint16_t)d0);
    wr16(m, a4 + 0x090u, (uint16_t)d1);
    if ((uint16_t)d4 <= 3u)                                /* 0x0C054E bhi */
        d4 = (uint16_t)(d4 ^ 1u);
    h_BFA46(m, rd32(m, a4 + 0x09Cu), (uint16_t)d4, rd16(m, a4 + 0x06Au), &x, &y);
    d0 = (uint16_t)x;
    d1 = (uint16_t)y;
    d2 = rd16(m, a4 + 0x0E6u);
    if ((int16_t)d2 > (int16_t)rd16(m, a4 + 0x0EEu))
        d2 = rd16(m, a4 + 0x0EEu);
    if ((int16_t)d0 < (int16_t)d2)
        d0 = d2;
    d2 = rd16(m, a4 + 0x0E8u);
    if ((int16_t)d2 < (int16_t)rd16(m, a4 + 0x0F0u))
        d2 = rd16(m, a4 + 0x0F0u);
    if ((int16_t)d0 > (int16_t)d2)
        d0 = d2;
    wr16(m, a4 + 0x092u, (uint16_t)d0);
    wr16(m, a4 + 0x094u, (uint16_t)d1);
    *d4p = d4;
}

/* ---- the state-chain handlers, all "return Z" (1 = keep walking) ---- */

/* 0x0C2922: tst.w $32(a4) ; spl.b d0 ; ext.w d0 - SPL sets 0xFF when the
 * value is minus-free, so the "commit" (NZ) case is a negative word. */
static int h_C2922(const f3mem_t *m, uint32_t a4)
{
    return (int16_t)rd16(m, a4 + 0x032u) < 0;
}

/* 0x0C2904: tst.w $30(a4) ; spl.b d0 ; ext.w d0 */
static int h_C2904(const f3mem_t *m, uint32_t a4)
{
    return (int16_t)rd16(m, a4 + 0x030u) < 0;
}

/* 0x0C29C0: tst.w $6a(a4) ; sne ; tst.w $36(a4,d0) ; smi ; ext.w */
static int h_C29C0(const f3mem_t *m, uint32_t a4)
{
    uint32_t sel = (rd16(m, a4 + 0x06Au) != 0u) ? 0xFFFFu : 0u;

    return (int16_t)rd16(m, a4 + 0x036u + (uint32_t)(int32_t)(int16_t)sel) >= 0;
}

/* 0x0C2A0C: tst.w $38(a4) ; seq.b d0 ; ext.w d0 */
static int h_C2A0C(const f3mem_t *m, uint32_t a4)
{
    return rd16(m, a4 + 0x038u) != 0u;
}

/* 0x0C2A16: tst.w $38(a4) ; spl.b d0 ; ext.w d0 */
static int h_C2A16(const f3mem_t *m, uint32_t a4)
{
    return (int16_t)rd16(m, a4 + 0x038u) < 0;
}

/* 0x0C29D8: blocked-cell scan on the 0x1E-byte ring at A4+0x0E. */
static int h_C29D8(const f3mem_t *m, uint32_t a4)
{
    uint32_t sel = (rd16(m, a4 + 0x06Au) != 0u) ? 0xFFFFu : 0u;
    uint32_t d2 = 3u + (uint16_t)sel;
    uint16_t d0, d1 = 0x0000u;

    if (rd16(m, a4 + 0x036u + (uint32_t)(int32_t)(int16_t)sel) != 0u)
        return 1;                                  /* 0x0C29E4 bne $c28e4 (d0 = 0) */
    d1 = (uint16_t)(0u - 24u);                     /* 0x0C29EC moveq #$e8,d1 */
    d0 = (uint16_t)(rd16(m, a4 + 0x00Cu) + d1);    /* 0x0C29EE..0x0C29F2 */
    for (;;) {
        d0 = (uint16_t)(d0 & 0x001Eu);             /* 0x0C29F4 */
        if ((rd8(m, a4 + 0x00Eu + (uint32_t)(int32_t)(int16_t)d0) &
             (uint8_t)(1u << (d2 & 7u))) != 0u)
            return 0;                              /* 0x0C29FC bne $c28e8 */
        d0 = (uint16_t)(d0 + 2u);                  /* 0x0C2A00 */
        d1 = (uint16_t)(d1 + 2u);                  /* 0x0C2A02 */
        if ((int16_t)d1 >= 0)
            break;                                 /* 0x0C2A04 bmi */
    }
    return 1;                                      /* 0x0C2A06 rts, Z = (d1 == 0) */
}

/* 0x0C2776: three-cell solidity probe in front of the object; NZ (0xFFFF)
 * when any of the three cells is blocked. */
static int h_C2776(const f3mem_t *m, uint32_t a4, uint32_t a5)
{
    uint32_t d0 = 0u, d1 = 0u, d2, d3, a0 = 0u;
    int clear;

    d2 = rd16(m, a4 + 0x0E2u);
    d3 = rd16(m, a4 + 0x004Eu);
    /* 0x0C277E move.w $c2770(pc,d3.w*2),d3: extension 0x32F0 has scale 2 */
    d3 = rd16(m, 0x000C2770u + (uint32_t)(int16_t)d3 * 2u);
    d2 = (uint16_t)(d2 + d3);
    if (rd16(m, a4 + 0x06Au) != 0u) {               /* 0x0C2784 */
        d2 = (uint16_t)(d2 - d3);
        d2 = (uint16_t)(d2 - d3);
    }
    d3 = rd16(m, a4 + 0x0DCu);
    if ((int16_t)d3 < (int16_t)rd16(m, a4 + 0x0E0u))          /* 0x0C2792 bge */
        d3 = rd16(m, a4 + 0x0E0u);
    d2 = (uint16_t)(d2 + rd16(m, a4 + 0x0A0u));     /* 0x0C279C */
    d3 = (uint16_t)(d3 + rd16(m, a4 + 0x0A4u));     /* 0x0C27A0 */
    d3 = (uint16_t)(d3 - 1u);                       /* 0x0C27A4 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)d2;
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d3 - 1u);  /* 0x0C27AA */
    clear = h_BD546(m, a5, &d0, &d1, &a0);          /* 0x0C27AC */
    if (!clear)
        return 0;                                   /* 0x0C27B2 bne $c27c4 -> -1 */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)d2;
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)d3;         /* 0x0C27B4..0x0C27B6 */
    clear = h_BD546(m, a5, &d0, &d1, &a0);          /* 0x0C27B8 */
    if (!clear)
        return 0;                                   /* 0x0C27C0 moveq #$ff */
    d0 = (d0 & 0xFFFF0000u) | (uint16_t)d2;
    d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d3 + 2u);  /* 0x0C27C8..0x0C27CC */
    clear = h_BD546(m, a5, &d0, &d1, &a0);          /* 0x0C27CE */
    if (!clear)
        return 0;                                   /* 0x0C27D8 moveq #$ff */
    return 1;                                       /* 0x0C27DC moveq #0 -> Z */
}

/* 0x0C2750: commit when the front cells are free of the shared map. */
static int h_C2750(const f3mem_t *m, uint32_t a4, uint32_t a5)
{
    if (!h_C29C0(m, a4))                            /* 0x0C2756 bne $c2762 */
        return 0;                                   /* 0x0C2762 moveq #$ff */
    if (!h_C2776(m, a4, a5))                        /* 0x0C2758 jsr $c2776 */
        return 1;                                   /* not.w d0 -> 0 -> Z=1 */
    return 0;                                       /* not.w 0 -> $ffff -> NZ */
}

/* 0x0C2A68: ring scan keyed on the long at A4+0x38. */
static int h_C2A68(const f3mem_t *m, uint32_t a4)
{
    uint32_t d0, d1;

    d0 = rd32(m, a4 + 0x038u);
    if (d0 == 0u)
        return 0;                                   /* 0x0C2A6C beq $c28e8 -> -1 */
    if ((uint16_t)d0 == 0u) {
        /* 0x0C2A9E */
        d1 = (uint16_t)(0u - 4u);
        d0 = (uint16_t)(rd16(m, a4 + 0x00Cu) + d1);
        for (;;) {
            d0 = (uint16_t)(d0 & 0x001Eu);
            if ((rd8(m, a4 + 0x00Eu + (uint32_t)(int32_t)(int16_t)d0) & 0x10u) != 0u)
                return 0;                           /* 0x0C2AB0 bne $c28e8 */
            d0 = (uint16_t)(d0 + 2u);
            d1 = (uint16_t)(d1 + 2u);
            if ((int16_t)d1 > 0)
                break;                              /* 0x0C2AB8 ble */
        }
        return 1;                                   /* 0x0C2ABA moveq #0 */
    }
    d0 = swap32(d0);
    if ((uint16_t)d0 != 0u)
        return 1;                                   /* 0x0C2A7A bne $c28e4 -> 0 */
    d1 = (uint16_t)(0u - 4u);
    d0 = (uint16_t)(rd16(m, a4 + 0x00Cu) + d1);
    for (;;) {
        d0 = (uint16_t)(d0 & 0x001Eu);
        if ((rd8(m, a4 + 0x00Eu + (uint32_t)(int32_t)(int16_t)d0) & 0x20u) != 0u)
            return 0;                               /* 0x0C2A90 bne $c28e8 */
        d0 = (uint16_t)(d0 + 2u);
        d1 = (uint16_t)(d1 + 2u);
        if ((int16_t)d1 > 0)
            break;                                  /* 0x0C2A98 ble */
    }
    return 1;                                       /* 0x0C2A9A moveq #0 */
}

/* 0x0BFED0 */
static int h_BFED0(const f3mem_t *m, uint32_t a4)
{
    if (rd16(m, a4 + 0x846u) == 0u)
        return 1;                                   /* 0x0BFEDC moveq #0 -> Z=1 */
    return h_C2A0C(m, a4);                          /* 0x0BFED6 jmp $c2a0c */
}

/* 0xC0068 */
static int h_C0068(const f3mem_t *m, uint32_t a4)
{
    if ((int16_t)rd16(m, a4 + 0x106u) <= 0)
        return 1;                                   /* 0x0C0070 moveq #0 */
    return h_C2A68(m, a4);                          /* 0x0C006C bgt $c2a68 */
}

/* 0xBFF86 */
static int h_BFF86(const f3mem_t *m, uint32_t a4)
{
    if ((int16_t)rd16(m, a4 + 0x108u) >= 0x0B)      /* 0x0BFF8C bge $bff9a */
        return 1;
    if (h_C2A0C(m, a4))                             /* 0x0BFF8E jsr $c2a0c ; beq */
        return 1;
    return 0;                                       /* 0x0BFF96 moveq #$ff */
}

/* 0xBFF10 */
static int h_BFF10(const f3mem_t *m, uint32_t a4)
{
    if (h_C2904(m, a4))                             /* 0x0BFF16 beq $bff9a */
        return 1;
    /* falls into 0xBFF1A */
    if (rd16(m, a4 + 0x0B2u) != 1u)                 /* 0x0BFF20 bne $bff9a */
        return 1;
    if ((int16_t)rd16(m, a4 + 0x108u) >= 0x0B)      /* 0x0BFF28 bge $bff9a */
        return 1;
    return h_C2A16(m, a4);                          /* 0x0BFF2A jmp $c2a16 */
}

/* 0xBFF1A */
static int h_BFF1A(const f3mem_t *m, uint32_t a4)
{
    if (rd16(m, a4 + 0x0B2u) != 1u)
        return 1;
    if ((int16_t)rd16(m, a4 + 0x108u) >= 0x0B)
        return 1;
    return h_C2A16(m, a4);
}

/* 0xBFF6C */
static int h_BFF6C(const f3mem_t *m, uint32_t a4)
{
    if (rd16(m, a4 + 0x0B2u) != 2u)                 /* 0x0BFF72 bne $bff9a */
        return 1;
    if (h_C2904(m, a4))                             /* 0x0BFF7A beq $bff9a */
        return 1;
    return h_BFF86(m, a4);                          /* 0x0BFF7C bra $bff86 */
}

/* 0xBFF7E */
static int h_BFF7E(const f3mem_t *m, uint32_t a4)
{
    if (rd16(m, a4 + 0x0B2u) != 2u)                 /* 0x0BFF84 bne $bff9a */
        return 1;
    return h_BFF86(m, a4);
}

/* 0xC0080 */
static int h_C0080(const f3mem_t *m, uint32_t a4)
{
    return rd16(m, a4 + 0x03Au) != 0u;              /* seq.b + ext.w */
}

/* Dispatch on the state-record handler pointer; 1 = keep walking, 0 = commit,
 * -1 = a handler this draft does not translate. */
static int chain_step(const f3mem_t *m, uint32_t a4, uint32_t a5, uint32_t addr)
{
    switch (addr) {
    case 0x000C2922u: return h_C2922(m, a4);
    case 0x000C29D8u: return h_C29D8(m, a4);
    case 0x000C2750u: return h_C2750(m, a4, a5);
    case 0x000BFED0u: return h_BFED0(m, a4);
    case 0x000C0068u: return h_C0068(m, a4);
    case 0x000BFF10u: return h_BFF10(m, a4);
    case 0x000BFF1Au: return h_BFF1A(m, a4);
    case 0x000BFF6Cu: return h_BFF6C(m, a4);
    case 0x000BFF7Eu: return h_BFF7E(m, a4);
    case 0x000BFF74u: return h_C2904(m, a4);       /* jsr $c2904 ; beq ; bra $bff86 */
    case 0x000BFF86u: return h_BFF86(m, a4);
    case 0x000C0080u: return h_C0080(m, a4);
    default: return -1;
    }
}

int Ea_0BFC56(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t sp0 = r->a[7];
    uint32_t a4 = r->a[4];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2];
    uint32_t d3 = r->d[3], d4 = r->d[4], d5 = r->d[5];
    uint32_t a0 = r->a[0], a3 = r->a[3];

    /* 0x0BFC56 movem.l d0-d5/a0/a3,-(a7) */
    r->a[7] = sp0 - 32u;
    wr32(m, sp0 - 32u, d0);
    wr32(m, sp0 - 28u, d1);
    wr32(m, sp0 - 24u, d2);
    wr32(m, sp0 - 20u, d3);
    wr32(m, sp0 - 16u, d4);
    wr32(m, sp0 - 12u, d5);
    wr32(m, sp0 - 8u, a0);
    wr32(m, sp0 - 4u, a3);

    /* 0x0BFC5A tst.l $9c(a4) ; 0x0BFC5E ble.w $bfec2 */
    if ((int32_t)rd32(m, a4 + 0x09Cu) <= 0)
        goto ret0;
    /* 0x0BFC62 lea.l ([$72,a4],$4),a3 */
    a3 = rd32(m, a4 + 0x072u) + 4u;
    /* 0x0BFC6A tst.w $7a(a4) ; beq.b $bfc7c */
    if (rd16(m, a4 + 0x07Au) != 0u) {
        /* 0x0BFC70 bftst ([$9c,a4],$20){11:1} ; bne.b $bfcb0 - the frame record's
         * flag bit 4 at (*(A4+0x9C))+0x21 (capture-calibrated: {11:1} reads
         * one bit past sub-field 11, i.e. byte 1, mask 0x10). */
        if ((rd8(m, rd32(m, a4 + 0x09Cu) + 0x21u) & 0x10u) != 0u)
            goto LFCB0;
    }
    for (;;) {
        /* 0x0BFC7C lea.l $a(a3),a3 */
        a3 += 0xAu;
        /* 0x0BFC80 tst.w (a3) ; 0x0BFC82 bmi.w $bfec2 */
        if ((int16_t)rd16(m, a3) < 0)
            goto ret0;
        /* 0x0BFC86 jsr ([a3]) ; 0x0BFC8A beq.b $bfc7c */
        {
            int z = chain_step(m, a4, a5, rd32(m, a3));

            if (z < 0)
                return -1;
            if (z)
                continue;
        }
        /* 0x0BFC8C move.l $4(a3),$76(a4) */
        wr32(m, a4 + 0x076u, rd32(m, a3 + 4u));
        wr16(m, a4 + 0x07Au, 0xFFFFu);              /* 0x0BFC92 */
        if ((rd16(m, a3 + 8u) & 2u) != 0u)          /* 0x0BFC98..0x0BFCA0 */
            break;
        if ((rd8(m, rd32(m, a4 + 0x09Cu) + 0x21u) & 0x10u) != 0u) /* 0x0BFCA2 */
            break;
        goto ret0;                                  /* 0x0BFCAC beq.w $bfec2 */
    }

LFCB0:
    /* 0x0BFCB0..0x0BFCC6 */
    wr16(m, a4 + 0x0A8u, rd16(m, a4 + 0x06Au));
    wr32(m, a4 + 0x0AAu, rd32(m, a4 + 0x09Cu));
    if (rd16(m, a4 + 0x07Cu) != 0u)
        wr16(m, a4 + 0x06Au, (uint16_t)(rd16(m, a4 + 0x06Au) ^ 1u));
    wr16(m, a4 + 0x07Cu, 0u);                       /* 0x0BFCC8 */
    a0 = rd32(m, rd32(m, a4 + 0x072u));             /* 0x0BFCCC movea.l ([$72,a4]),a0:
                                                     * memory indirect = a double load */
    d0 = (uint16_t)rd16(m, a4 + 0x082u);            /* 0x0BFCD2 */
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)d0 * 0x10u); /* 0x0BFCD6 muls.w #$10 */
    a0 += (uint32_t)(int32_t)(int16_t)(uint16_t)d0; /* 0x0BFCDA adda.w d0,a0 */
    d0 = rd32(m, a0 + 0x0Cu) & 0x0Fu;               /* 0x0BFCDC..0x0BFCE0 */
    d1 = rd16(m, a3 + 8u) & 1u;                     /* 0x0BFCE6..0x0BFCEA */
    if (d1 != 0u)
        d0 = 0u;                                    /* 0x0BFCF0 moveq #0 */
    wr16(m, a4 + 0x080u, 0xFFFFu);                  /* 0x0BFCF2 */
    a0 = rd32(m, a4 + 0x076u);                      /* 0x0BFCF8 movea.l $76(a4),a0 */
    wr32(m, a4 + 0x072u, a0);                       /* 0x0BFCFC */
    wr32(m, a4 + 0x076u, rd32(m, a0 + 8u));         /* 0x0BFD00 */
    wr16(m, a4 + 0x082u, 0u);                       /* 0x0BFD06 */
    a0 = rd32(m, rd32(m, a4 + 0x072u));             /* 0x0BFD0A movea.l ([$72,a4]),a0 */
    wr16(m, a4 + 0x07Eu, 0xFFFFu);                  /* 0x0BFD10 */
    wr16(m, a4 + 0x834u, 0u);                       /* 0x0BFD16 */
    wr16(m, a4 + 0x07Au, 0u);                       /* 0x0BFD1A */
    wr16(m, a4 + 0x082u, (uint16_t)(rd16(m, a4 + 0x082u) - (uint16_t)d0)); /* 0x0BFD1E */
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)d0 * 0x10u);     /* 0x0BFD22 */
    a0 -= (uint32_t)(int32_t)(int16_t)(uint16_t)d0; /* 0x0BFD26 suba.w d0,a0 */
    wr32(m, a4 + 0x0AEu, a0);                       /* 0x0BFD28 */
    d1 = (uint16_t)(rd16(m, a4 + 0x0B2u) << 2);     /* 0x0BFD2C..0x0BFD30 */
    a0 = rd32(m, a0 + (uint32_t)(int32_t)(int16_t)d1);  /* 0x0BFD32 movea.l (a0,d1.w),a0 */
    wr32(m, a4 + 0x09Cu, a0);                       /* 0x0BFD36 */
    /* 0x0BFD3A bftst $20(a0){10:1} ; sne.b d0 ; andi.w #$1,d0 ; or.w d0,$7c(a4) */
    d0 = ((rd8(m, a0 + 0x21u) & 0x08u) != 0u) ? 1u : 0u;
    wr16(m, a4 + 0x07Cu, (uint16_t)(rd16(m, a4 + 0x07Cu) | (uint16_t)d0));
    wr16(m, a4 + 0x084u, rd16(m, a0 + 4u));         /* 0x0BFD4A */
    d4 = (uint16_t)rd16(m, rd32(m, a4 + 0x09Cu) + 8u); /* 0x0BFD50 */
    if ((uint16_t)d4 <= 3u) {                       /* 0x0BFD58 bhi */
        if (rd16(m, a4 + 0x06Au) != 0u)
            d4 = (uint16_t)(d4 ^ 1u);
    }
    /* 0x0BFD68..0x0BFD96: the old and new cell origins */
    {
        uint32_t x1, y1, x2, y2;

        h_BFA46(m, rd32(m, a4 + 0x0AAu), (uint16_t)d4, rd16(m, a4 + 0x0A8u), &x1, &y1);
        h_BFA46(m, rd32(m, a4 + 0x09Cu), (uint16_t)d4, rd16(m, a4 + 0x06Au), &x2, &y2);
        d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)x1 - (uint16_t)x2);
        d1 = (uint32_t)(int32_t)(int16_t)(uint16_t)((uint16_t)y1 - (uint16_t)y2);
    }
    wr16(m, a4 + 0x0A0u, (uint16_t)(rd16(m, a4 + 0x0A0u) + (uint16_t)d0)); /* 0x0BFD98 */
    wr16(m, a4 + 0x0A4u, (uint16_t)(rd16(m, a4 + 0x0A4u) + (uint16_t)d1)); /* 0x0BFD9C */
    /* 0x0BFDA0 tst.w ([$72,a4],$fffe) ; beq.w $bfe34 */
    if (rd16(m, rd32(m, a4 + 0x072u) - 2u) == 0u)
        goto LFE34;
    d5 = (uint16_t)(0x18u + rd16(m, a4 + 0x0A4u));  /* 0x0BFDAC..0x0BFDAE */
    d4 = 2u;                                        /* 0x0BFDB2 moveq #$2,d4 */
    {
        uint32_t x, y;

        h_BFA46(m, rd32(m, a4 + 0x09Cu), (uint16_t)d4, rd16(m, a4 + 0x06Au), &x, &y);
        d2 = (uint16_t)((uint16_t)x - 1u);          /* 0x0BFDCA subq.w #1,d3 */
        d2 = (uint16_t)(d2 + rd16(m, a4 + 0x0A0u)); /* 0x0BFDCC */
        d3 = (uint16_t)((uint16_t)y - 1u);
        d3 = (uint16_t)(d3 + rd16(m, a4 + 0x0A4u)); /* 0x0BFDD0 */
    }
    d1 = d5;                                        /* 0x0BFDD4 */
    d0 = d2;                                        /* 0x0BFDD6 */
    h_BD9F2(m, a5, &d0, &d1, &a0);                  /* 0x0BFDD8 */
    d1 = (uint16_t)(rd16(m, a0 + 2u) - rd16(m, a0)); /* 0x0BFDDE..0x0BFDE2 */
    if ((uint16_t)d1 <= 8u)                         /* 0x0BFDE4 bls.w $bfeae */
        goto LFEAE;
    d2 = (uint16_t)d0;                              /* 0x0BFDEC move.w d0,d2 */
    if ((uint16_t)d4 <= 3u)                         /* 0x0BFDEE bhi */
        d4 = (uint16_t)(d4 ^ 1u);
    {
        uint32_t x, y;

        h_BFA46(m, rd32(m, a4 + 0x09Cu), (uint16_t)d4, rd16(m, a4 + 0x06Au), &x, &y);
        d0 = (uint16_t)((uint16_t)x + rd16(m, a4 + 0x0A0u)); /* 0x0BFE0E */
        d1 = (uint16_t)y;
    }
    d1 = d5;                                        /* 0x0BFE12 */
    h_BD9F2(m, a5, &d0, &d1, &a0);                  /* 0x0BFE14 */
    d1 = (uint16_t)(rd16(m, a0 + 2u) - rd16(m, a0)); /* 0x0BFE1A..0x0BFE1E */
    if ((uint16_t)d1 <= 8u)                         /* 0x0BFE24 bls */
        goto LFEAE;
    if ((int16_t)d0 > (int16_t)d2)                   /* 0x0BFE28 cmp.w d2,d0 ; ble $bfea0 */
        d0 = d2;                                    /* 0x0BFE2E move.w d2,d0 */
    goto LFEA0;

LFE34:
    /* 0x0BFE34 */
    d3 = (uint16_t)(d3 - 1u);                       /* 0x0BFE34 subq.w #1,d3 */
    d2 = (uint16_t)(d2 + rd16(m, a4 + 0x0A0u));     /* 0x0BFE36 */
    d3 = (uint16_t)(d3 + rd16(m, a4 + 0x0A4u));     /* 0x0BFE3A */
    d1 = d3;                                        /* 0x0BFE3E */
    d0 = d2;                                        /* 0x0BFE40 */
    d1 = (uint16_t)(d1 - 4u);                       /* 0x0BFE42 subq.w #4,d1 */
    h_BD9F2(m, a5, &d0, &d1, &a0);                  /* 0x0BFE44 */
    d1 = (uint16_t)(rd16(m, a0 + 2u) - rd16(m, a0)); /* 0x0BFE4A..0x0BFE4E */
    if ((uint16_t)d1 <= 8u)                         /* 0x0BFE50 bls */
        goto LFEAE;
    d2 = (uint16_t)d0;                              /* 0x0BFE58 */
    if ((uint16_t)d4 <= 3u)                         /* 0x0BFE5A bhi */
        d4 = (uint16_t)(d4 ^ 1u);
    {
        uint32_t x, y;

        h_BFA46(m, rd32(m, a4 + 0x09Cu), (uint16_t)d4, rd16(m, a4 + 0x06Au), &x, &y);
        d0 = (uint16_t)x;
        d1 = (uint16_t)((uint16_t)y - 3u);          /* 0x0BFE7A subq.w #3,d1 */
        d0 = (uint16_t)(d0 + rd16(m, a4 + 0x0A0u)); /* 0x0BFE7C */
        d1 = (uint16_t)(d1 + rd16(m, a4 + 0x0A4u)); /* 0x0BFE80 */
        d1 = (uint16_t)(d1 - 4u);                   /* 0x0BFE84 */
    }
    h_BD9F2(m, a5, &d0, &d1, &a0);                  /* 0x0BFE86 */
    d1 = (uint16_t)(rd16(m, a0 + 2u) - rd16(m, a0)); /* 0x0BFE8C..0x0BFE90 */
    if ((uint16_t)d1 <= 8u)                         /* 0x0BFE96 bls */
        goto LFEAE;
    if ((int16_t)d0 > (int16_t)d2)                   /* 0x0BFE9A cmp.w d2,d0 ; ble $bfea0 */
        d0 = d2;

LFEA0:
    /* 0x0BFEA0 sub.w d3,d0 ; cmp2.w $bfeca(pc),d0 ; bcs.b $bfeae */
    d0 = (uint16_t)(d0 - (uint16_t)d3);
    if (!outside(m, 0x000BFECAu, (uint16_t)d0))
        wr16(m, a4 + 0x0A4u, (uint16_t)(rd16(m, a4 + 0x0A4u) + (uint16_t)d0));
LFEAE:
    /* 0x0BFEAE bsr.w $bfa96 ; 0x0BFEB2 bsr.w $c019e ; 0x0BFEB6 bsr.w $c04a2 */
    h_BFA96(m, a4);
    h_C019E(m, a4, &d0, &d1, &d2, &d3, &d4, &a0);
    h_C04A2(m, a4, &d4);
    d0 = 0xFFFFFFFFu;                               /* 0x0BFEBA moveq #$ff,d0 */
    goto restore;

ret0:
    d0 = 0u;                                        /* 0x0BFEC2 moveq #$0,d0 */
restore:
    /* 0x0BFEBC/0x0BFEC4 movem.l (a7)+,d0-d5/a0/a3 - D0-D5 and A0/A3 come back
     * from the frame, so the moveq above is not visible to the caller. */
    d0 = rd32(m, sp0 - 32u);
    d1 = rd32(m, sp0 - 28u);
    d2 = rd32(m, sp0 - 24u);
    d3 = rd32(m, sp0 - 20u);
    d4 = rd32(m, sp0 - 16u);
    d5 = rd32(m, sp0 - 12u);
    a0 = rd32(m, sp0 - 8u);
    a3 = rd32(m, sp0 - 4u);
    r->d[0] = d0; r->d[1] = d1; r->d[2] = d2;
    r->d[3] = d3; r->d[4] = d4; r->d[5] = d5;
    r->a[0] = a0; r->a[3] = a3;
    r->a[7] = sp0 + 4u;                             /* 0x0BFEC8 rts */
    return 0;
}
