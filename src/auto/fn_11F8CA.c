/* ROM 0x11F8CA: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11F8CA(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F8CA: Decrement word at A0+6, branch if >0 to 0x11F8C2 */
    uint16_t val = rd16(m, r->a[0] + 6);
    val--;
    wr16(m, r->a[0] + 6, val);
    if ((int16_t)val > 0) {
        wr32(m, r->a[0], 0x0011F8CA);
        return 0;
    }

    /* ROM 0x11F8D0: Clear bit 2 of byte at A5+0x30EE (bfclr {0x5:0x1}) */
    uint8_t b = rd8(m, F3_A5 + 0x30ee);
    b &= ~0x04;
    wr8(m, F3_A5 + 0x30ee, b);

    /* ROM 0x11F8D6: Branch to 0x11F7B6 */
    wr32(m, r->a[0], 0x0011F7BE);
    return 0;
}
