/* ROM 0x121B60: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_121B60(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x121B60: Decrement word at A0+6 */
    uint16_t v = rd16(m, r->a[0] + 6u);
    v--;
    wr16(m, r->a[0] + 6u, v);

    /* ROM 0x121B64: Branch if > 0 to 0x121B58 */
    if ((int16_t)v > 0) {
        /* ROM 0x121B58: Store 0x121B60 at A0 and return */
        wr32(m, r->a[0], 0x121B60u);
        return 0;
    }

    /* ROM 0x121B66: bfset (0x30ee,A5){0xa:0x1} -> offset 0x2 (bit 5) */
    uint8_t attr = rd8(m, F3_A5 + 0x30EEu);
    attr |= (1u << (7 - 0x2u));
    /* ROM 0x121B6C: bfset (0x30ee,A5){0x1:0x1} -> offset 0x1 (bit 6) */
    attr |= (1u << (7 - 0x1u));
    wr8(m, F3_A5 + 0x30EEu, attr);

    /* ROM 0x121B72: Store 0x3C to word at A0+6 */
    wr16(m, r->a[0] + 6u, 0x3Cu);

    /* ROM 0x121B78: Store 0x121B80 to long at A0 */
    wr32(m, r->a[0], 0x121B80u);

    /* ROM 0x121B7E: RTS */
    return 0;
}
