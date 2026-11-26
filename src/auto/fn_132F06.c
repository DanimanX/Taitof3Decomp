/* ROM 0x132F06: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_132F06(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x132F06: subq.w #1, (0x6,A0) */
    uint16_t val = (uint16_t)rd16(m, r->a[0] + 6u);
    val -= 1u;
    wr16(m, r->a[0] + 6u, val);

    /* ROM 0x132F0A: bgt.b 0x132EFE */
    if ((int16_t)val > 0) {
        /* ROM 0x132EFE: move.l #0x132f06,(A0) */
        wr32(m, r->a[0], 0x00132F06u);
        /* ROM 0x132F04: rts */
        return 0;
    }

    /* ROM 0x132F0C: move.b #0x3,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 0x03u);

    /* ROM 0x132F12: bfset (0x30ee,A5){0xc:0x1} */
    {
        uint8_t v = rd8(m, F3_A5 + 0x30EEu);
        v |= (1u << (7u - (0xCu & 7u)));
        wr8(m, F3_A5 + 0x30EEu, v);
    }

    /* ROM 0x132F18: move.w #0xb4,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x00B4u);

    /* ROM 0x132F1E: beq.b 0x132F2E (never taken, move doesn't affect flags) */
    /* falls through to 0x132F20 */

    /* ROM 0x132F20: move.l #0x132f28,(A0) */
    wr32(m, r->a[0], 0x00132F28u);
    /* ROM 0x132F26: rts */
    return 0;
}
