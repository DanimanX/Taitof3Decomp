/* ROM 0x121C60: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_121C60(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x121C60: subq.w #0x1,(0x6,A0) */
    uint16_t val = rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);
    if ((int16_t)val > 0) {
        /* ROM 0x121C58: move.l #0x121c60,(A0) */
        wr32(m, r->a[0], 0x121C60u);
        return 0;
    }

    /* ROM 0x121C66: bfset (0x30ee,A5){0x4:0x1} */
    uint8_t byte = rd8(m, F3_A5 + 0x30EEu);
    byte |= 0x08u;
    wr8(m, F3_A5 + 0x30EEu, byte);

    /* ROM 0x121C6C: move.w #-0x8000,(0x20,A0) */
    wr16(m, r->a[0] + 0x20u, 0x8000u);

    /* ROM 0x121C72: move.w #0x3,(0x42,A0) */
    wr16(m, r->a[0] + 0x42u, 0x3u);

    /* ROM 0x121C78: move.w #0x300,(0x6,A0) */
    wr16(m, r->a[0] + 0x6u, 0x300u);

    /* ROM 0x121C7E: move.l #0x121c86,(A0) */
    wr32(m, r->a[0], 0x121C86u);

    /* ROM 0x121C84: rts */
    return 0;
}
