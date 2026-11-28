/* ROM 0x1316B4: one step of the alternating playfield rows of the stage
 * transition. On even steps (bit 1 of A5+0x34EC) it zeroes the 0x62A030 row
 * and fills 0x62B430/0x62B630/0x627430 with 0x300B/0x700C/0x0300; on odd
 * steps 0x62A030 gets 0x5000 (0xB000 on the second field) and 0x627430 gets
 * 0xF300. One word per row, forwards or backwards on the second field. */
#include "ea_auto.h"

/* tst.w -$7ec2(a5): non-zero on the second interlace field, where every row
 * is written backwards from its mirror address instead. */
#define EA_ODD 0x7EC2u

/* Store `cnt`+1 copies of `val` into the row at fwd, or backwards from back.
 * dbra leaves D1 = 0xFFFF; the row pointer ends in A0. */
static void ea_fill(const f3mem_t *m, f3regs_t *r, uint32_t fwd, uint32_t back,
                    uint16_t val, uint16_t cnt)
{
    uint32_t a5 = r->a[5];
    uint32_t a0;
    unsigned n = (unsigned)(uint16_t)(cnt + 1u);

    if (rd16(m, a5 - EA_ODD) == 0u) {              /* tst.w ; bne */
        a0 = fwd;
        for (unsigned i = 0; i < n; i++) {         /* move.w d2,(a0)+ ; dbra */
            wr16(m, a0, val);
            a0 += 2u;
        }
    } else {
        a0 = back;
        for (unsigned i = 0; i < n; i++) {         /* move.w d2,-(a0) ; dbra */
            a0 -= 2u;
            wr16(m, a0, val);
        }
    }
    r->d[1] = set_word(r->d[1], 0xFFFFu);
    r->d[2] = set_word(r->d[2], val);
    r->a[0] = a0;
}

int Ea_1316B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t a5 = r->a[5];
    uint32_t sp = r->a[7];

    r->d[0] = set_word(r->d[0], rd16(m, a5 + 0x34ECu));       /* 0x1316B4 */
    r->d[0] = set_word(r->d[0], (uint16_t)((uint16_t)r->d[0] >> 2)); /* 0x1316B8 asr.w #2 */
    r->d[0] = set_word(r->d[0], (uint16_t)(r->d[0] & 0x0001u));      /* 0x1316BA */
    if ((uint16_t)r->d[0] != 0u) {                 /* 0x1316BE bne.w 0x131796 */
        /* odd step */
        uint16_t v = rd16(m, a5 - EA_ODD) != 0u ? 0xB000u : 0x5000u; /* 0x131796 */
        r->d[0] = set_word(r->d[0], v);            /* 0x13179E / 0x1317A6 */
        ea_fill(m, r, 0x0062A030u, 0x0062A1D0u, v, 0u);      /* 0x1317AA */
        ea_fill(m, r, 0x0062B430u, 0x0062B5D0u, 0x300Bu, 0u); /* 0x1317DA */
        ea_fill(m, r, 0x0062B630u, 0x0062B7D0u, 0x700Cu, 0u); /* 0x13180E */
        ea_fill(m, r, 0x00627430u, 0x006275D0u, 0xF300u, 0u); /* 0x131842 */
        r->a[7] = sp + 4u;                         /* 0x131876 rts */
        return 0;
    }

    /* even step */
    r->d[0] = set_word(r->d[0], 0x0000u);          /* 0x1316C2 move.w #$0,d0 */
    ea_fill(m, r, 0x0062A030u, 0x0062A1D0u, 0x0000u, 0u); /* 0x1316C6 */
    ea_fill(m, r, 0x0062B430u, 0x0062B5D0u, 0x300Bu, 0u); /* 0x1316F6 */
    ea_fill(m, r, 0x0062B630u, 0x0062B7D0u, 0x700Cu, 0u); /* 0x13172A */
    ea_fill(m, r, 0x00627430u, 0x006275D0u, 0x0300u, 0u); /* 0x13175E */

    r->a[7] = sp + 4u;                             /* 0x131876 rts */
    return 0;
}
