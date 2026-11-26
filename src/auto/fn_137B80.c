/* ROM 0x137B80: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137B80(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137B80: Decrement word at A0+6, branch to LAB_00137b78 if > 0 */
    uint16_t val = rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);
    if ((int16_t)val > 0) {
        /* LAB_00137b78: Store 0x137B80 at A0, return */
        wr32(m, r->a[0], 0x00137B80u);
        return 0;
    }

    /* LAB_00137b86: bfset bit 0x10 at A5+0x30EE, store 0x78 at A0+6 */
    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b |= (1u << (0x10 & 7u));
    wr8(m, F3_A5 + 0x30EEu, b);
    wr16(m, r->a[0] + 6u, 0x0078u);

    /* beq checks Z flag from subq.w above. If Z=1 (val was 0), branch to LAB_00137ba2 */
    if (val == 0) {
        /* LAB_00137ba2: bfset bit 0x11 at A5+0x30EE, store 0x12C at A0+6 */
        b = rd8(m, F3_A5 + 0x30EEu);
        b |= (1u << (0x11 & 7u));
        wr8(m, F3_A5 + 0x30EEu, b);
        wr16(m, r->a[0] + 6u, 0x012Cu);
        /* LAB_00137bae: Store 0x137BB6 at A0, return */
        wr32(m, r->a[0], 0x00137BB6u);
        return 0;
    }

    /* LAB_00137b94: Store 0x137B9C at A0, return */
    wr32(m, r->a[0], 0x00137B9Cu);
    return 0;
}
