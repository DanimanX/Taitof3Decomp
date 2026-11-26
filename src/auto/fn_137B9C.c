/* ROM 0x137B9C: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

static void Fn_137B94(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137B94: Store 0x137B9C to A0 */
    wr32(m, r->a[0], 0x137B9C);
}

static void Fn_137BAE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137BAE: Store 0x137BB6 to A0 */
    wr32(m, r->a[0], 0x137BB6);
}

int Fn_137B9C(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137B9C: Decrement word at A0+6, branch if > 0 to 0x137B94 */
    uint16_t val = rd16(m, r->a[0] + 6);
    val--;
    wr16(m, r->a[0] + 6, val);
    if (val > 0) {
        Fn_137B94(m, r);
        return 0;
    }

    /* ROM 0x137BA2: bfset bit 1 of byte at A5+0x30EE */
    uint8_t byte = rd8(m, F3_A5 + 0x30EEu);
    byte |= (1u << (0x11 & 7));
    wr8(m, F3_A5 + 0x30EEu, byte);

    /* ROM 0x137BA8: Store 0x12C to A0+6 */
    wr16(m, r->a[0] + 6, 0x12C);

    /* ROM 0x137BAE: Store 0x137BB6 to A0 */
    Fn_137BAE(m, r);
    return 0;
}
