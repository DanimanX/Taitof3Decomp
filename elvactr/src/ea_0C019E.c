/* ROM 0x0C019E: per-frame target easing for the object at A4: optionally
 * reload the eight target words at A4+0xE6..0xF4 from the packed signed-byte
 * table *(A4+0x9C)+0x18, mirror the A4+0xE2/0xE4 box corners into
 * A4+0xF6..0x104, then move that box and its A4+0xA0-biased copy at
 * A4+0x83E..0x844 half of the remaining distance towards the targets. */
#include "ea_auto.h"

/* 0x0C01AE..0x0C01D4: move.b (a0)+,dX ; ext.w dX ; swap dX ;
 * move.b (a0)+,dX ; ext.w dX - the long is (sext16 b_hi)<<16 | (sext16 b_lo). */
static uint32_t pack_sb(uint8_t b_hi, uint8_t b_lo)
{
    int16_t hi = (int16_t)((b_hi & 0x80u) ? (int)b_hi - 0x100 : (int)b_hi);
    int16_t lo = (int16_t)((b_lo & 0x80u) ? (int)b_lo - 0x100 : (int)b_lo);

    return ((uint32_t)(uint16_t)hi << 16) | (uint32_t)(uint16_t)lo;
}

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
}

/* .w only: keep the high word, replace the low word. */
static uint32_t wneg(uint32_t d)
{
    return (d & 0xFFFF0000u) | (uint16_t)(0u - (d & 0xFFFFu));
}

static uint32_t wadd(uint32_t d, uint32_t v)
{
    return (d & 0xFFFF0000u) | (uint16_t)((d & 0xFFFFu) + (v & 0xFFFFu));
}

/* One easing step, e.g. 0x0C023A..0x0C0256:
 *   move.w src(a4),d0 ; [add.w $a0(a4),d0] ; sub.w dst(a4),d0 ; beq done
 *   bmi neg: asr.w #1,d0 ; add.w d0,dst(a4)
 *   pos    : neg.w d0 ; asr.w #1,d0 ; neg.w d0 ; add.w d0,dst(a4)
 * i.e. dst += diff rounded away from zero by one half; d0.w ends as that step. */
static uint32_t ease_word(const f3mem_t *m, uint32_t a4, uint32_t d0,
                          uint32_t src_off, uint32_t dst_off, uint16_t bias)
{
    uint16_t dst = rd16(m, a4 + dst_off);
    uint16_t diff = (uint16_t)((uint16_t)(rd16(m, a4 + src_off) + bias) - dst);
    uint16_t step = 0u;

    if (diff != 0u) {
        if ((diff & 0x8000u) == 0u)                       /* bmi not taken */
            step = (uint16_t)(((uint32_t)diff + 1u) / 2u); /* neg/asr/neg */
        else
            step = (uint16_t)(0u - (uint16_t)(((uint32_t)(0x10000u - diff) + 1u) / 2u));
        wr16(m, a4 + dst_off, (uint16_t)(dst + step));    /* add.w d0,dst(a4) */
    }
    return (d0 & 0xFFFF0000u) | step;
}

int Ea_0C019E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a4 = r->a[4];
    uint32_t d0 = r->d[0], d1 = r->d[1], d2 = r->d[2], d3 = r->d[3];
    uint32_t d4 = r->d[4];
    uint32_t a0 = r->a[0];

    /* 0x0C019E tst.w $80(a4) ; 0x0C01A2 beq.w $c023a - mode 0 keeps the
     * targets A4+0xE6..0xF4 as they are and goes straight to the easing. */
    if (rd16(m, a4 + 0x080u) != 0u) {
        /* 0x0C01A6 lea.l ([$9c,a4],$18),a0 - full-format memory indirect:
         * a0 = long (A4+0x9C) + word outer displacement 0x18. */
        a0 = rd32(m, a4 + 0x09Cu) + 0x18u;
        d0 = pack_sb(rd8(m, a0 + 0u), rd8(m, a0 + 1u)); /* 0x0C01AE..0x0C01B6 */
        d1 = pack_sb(rd8(m, a0 + 2u), rd8(m, a0 + 3u)); /* 0x0C01B8..0x0C01C0 */
        d2 = pack_sb(rd8(m, a0 + 4u), rd8(m, a0 + 5u)); /* 0x0C01C2..0x0C01CA */
        d3 = pack_sb(rd8(m, a0 + 6u), rd8(m, a0 + 7u)); /* 0x0C01CC..0x0C01D4 */
        a0 += 8u;                                       /* four (a0)+ pairs */

        /* 0x0C01D6 tst.w $6a(a4) ; 0x0C01DA beq.b $c0200 */
        if (rd16(m, a4 + 0x06Au) != 0u) {
            uint32_t p = rd32(m, a4 + 0x09Cu);          /* 0x0C01DC movea.l $9c(a4),a0 */
            a0 = p;
            /* d4 = ((word (a0+2)) + 1) << 4) - 1, word ops only */
            d4 = (d4 & 0xFFFF0000u) | (uint16_t)(rd16(m, p + 2u) + 1u); /* 0x0C01E2..0x0C01E6 */
            d4 = (d4 & 0xFFFF0000u) | (uint16_t)((uint16_t)(d4 & 0xFFFFu) << 4); /* 0x0C01E8 */
            d4 = (d4 & 0xFFFF0000u) | (uint16_t)((d4 & 0xFFFFu) - 1u); /* 0x0C01EA */
            /* 0x0C01EC..0x0C01FE: mirror d0/d2 inside a d4-sized cell */
            d0 = wadd(wneg(d0), d4);
            d2 = wadd(wneg(d2), d4);
            d0 = swap32(d0);
            d2 = swap32(d2);
            d0 = wadd(wneg(d0), d4);
            d2 = wadd(wneg(d2), d4);
        }
        wr32(m, a4 + 0x0E6u, d0);                       /* 0x0C0200 */
        wr32(m, a4 + 0x0EAu, d1);                       /* 0x0C0204 */
        wr32(m, a4 + 0x0EEu, d2);                       /* 0x0C0208 */
        wr32(m, a4 + 0x0F2u, d3);                       /* 0x0C020C */

        {
            uint16_t e2 = rd16(m, a4 + 0x0E2u);         /* 0x0C0210 move.w $e2(a4),d0 */
            uint16_t e4 = rd16(m, a4 + 0x0E4u);         /* 0x0C021E move.w $e4(a4),d1 */

            d0 = swap32((d0 & 0xFFFF0000u) | (uint16_t)(e2 - 3u)); /* 0x0C0214..0x0C0216 */
            d0 = (d0 & 0xFFFF0000u) | (uint16_t)(e2 + 3u);         /* 0x0C0218..0x0C021C */
            d1 = swap32((d1 & 0xFFFF0000u) | (uint16_t)(e4 - 6u)); /* 0x0C0222..0x0C0224 */
            d1 = (d1 & 0xFFFF0000u) | e4;                          /* 0x0C0226 */
        }
        wr32(m, a4 + 0x0F6u, d0);                       /* 0x0C022A */
        wr32(m, a4 + 0x0FAu, d1);                       /* 0x0C022E */
        wr32(m, a4 + 0x0FEu, d0);                       /* 0x0C0232 */
        wr32(m, a4 + 0x102u, d1);                       /* 0x0C0236 */
    }

    /* 0x0C023A..0x0C033A: eight half-distance steps, source A4+0xE6..0xF4
     * against the mirrored box A4+0xF6..0x104. */
    d0 = ease_word(m, a4, d0, 0x0E6u, 0x0F6u, 0u);      /* 0x0C023A */
    d0 = ease_word(m, a4, d0, 0x0E8u, 0x0F8u, 0u);      /* 0x0C025A */
    d0 = ease_word(m, a4, d0, 0x0EAu, 0x0FAu, 0u);      /* 0x0C027A */
    d0 = ease_word(m, a4, d0, 0x0ECu, 0x0FCu, 0u);      /* 0x0C029A */
    d0 = ease_word(m, a4, d0, 0x0EEu, 0x0FEu, 0u);      /* 0x0C02BA */
    d0 = ease_word(m, a4, d0, 0x0F0u, 0x100u, 0u);      /* 0x0C02DA */
    d0 = ease_word(m, a4, d0, 0x0F2u, 0x102u, 0u);      /* 0x0C02FA */
    d0 = ease_word(m, a4, d0, 0x0F4u, 0x104u, 0u);      /* 0x0C031A */

    /* 0x0C033A..0x0C03C6: the same step for the biased copy at A4+0x83E..0x844,
     * source A4+0xE6/E8/EE/F0 plus word $a0(a4). */
    d0 = ease_word(m, a4, d0, 0x0E6u, 0x83Eu, rd16(m, a4 + 0x0A0u)); /* 0x0C033A */
    d0 = ease_word(m, a4, d0, 0x0E8u, 0x840u, rd16(m, a4 + 0x0A0u)); /* 0x0C035E */
    d0 = ease_word(m, a4, d0, 0x0EEu, 0x842u, rd16(m, a4 + 0x0A0u)); /* 0x0C0382 */
    d0 = ease_word(m, a4, d0, 0x0F0u, 0x844u, rd16(m, a4 + 0x0A0u)); /* 0x0C03A6 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = d2;
    r->d[3] = d3;
    r->d[4] = d4;
    r->a[0] = a0;
    r->a[7] += 4u;                                      /* 0x0C03CA rts */
    return 0;
}
