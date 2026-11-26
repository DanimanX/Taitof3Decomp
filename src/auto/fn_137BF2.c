/* ROM 0x137BF2: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137BF2(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137BF2: cmpi.w #0x5, (0x32d6,A5) */
    uint16_t val = rd16(m, F3_A5 + 0x32d6u);
    /* ROM 0x137BF8: bge.w 0x137C36 */
    if ((int16_t)val >= 5) goto L_137C36;

    /* ROM 0x137BFC: subq.w #1, (0x6,A0) */
    uint16_t tmp = (uint16_t)(rd16(m, r->a[0] + 6u) - 1u);
    wr16(m, r->a[0] + 6u, tmp);
    /* ROM 0x137C00: bgt.b 0x137BEA */
    if ((int16_t)tmp > 0) {
        /* ROM 0x137BEA: move.l #0x137bf2,(A0) */
        wr32(m, r->a[0], 0x00137BF2u);
        /* ROM 0x137BF0: rts */
        return 0;
    }

    /* ROM 0x137C02: bfclr (0x30ee,A5){0xf:0x1} */
    uint16_t w = rd16(m, F3_A5 + 0x30EEu);
    wr16(m, F3_A5 + 0x30EEu, w & ~1u);

    /* ROM 0x137C08: move.w #0x78,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x0078u);

    /* ROM 0x137C0E: move.l #0x137c16,(A0) */
    wr32(m, r->a[0], 0x00137C16u);
    /* ROM 0x137C14: rts */
    return 0;

L_137C36:
    /* ROM 0x137C36: clr.w (0x3318,A5) */
    wr16(m, F3_A5 + 0x3318u, 0u);

    /* ROM 0x137C3A: move.b #2,(0x54,A0) */
    wr8(m, r->a[0] + 0x54u, 2u);

    /* ROM 0x137C40: move.w #0x138,(0x50,A0) */
    wr16(m, r->a[0] + 0x50u, 0x0138u);

    /* ROM 0x137C46: move.w #0x78,(0x52,A0) */
    wr16(m, r->a[0] + 0x52u, 0x0078u);

    /* ROM 0x137C4C: move.w #0x12c,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x012Cu);

    /* ROM 0x137C52: beq.b 0x137C74 (always taken after clr.w) */
    /* ROM 0x137C74: bfset (0x30ee,A5){0x12:0x1} */
    uint32_t l = rd32(m, F3_A5 + 0x30EEu);
    wr32(m, F3_A5 + 0x30EEu, l | (1u << 13));

    /* ROM 0x137C7A: move.w #0xc8,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x00C8u);

    /* ROM 0x137C80: beq.b 0x137C90 (always taken) */
    /* ROM 0x137C90: move.b #2,(0x54,A0) */
    wr8(m, r->a[0] + 0x54u, 2u);

    /* ROM 0x137C96: move.w #0x9f,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x009Fu);

    /* ROM 0x137C9C: move.l #0x137ca4,(A0) */
    wr32(m, r->a[0], 0x00137CA4u);
    /* ROM 0x137CA2: rts */
    return 0;
}
