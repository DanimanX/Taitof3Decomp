/* ROM 0x193F22: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_193F22(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x193F22: Decrement word at A0+6, branch to 0x193F1A if > 0 */
    uint16_t val = rd16(m, r->a[0] + 6);
    val--;
    wr16(m, r->a[0] + 6, val);
    if (val > 0) {
        wr32(m, r->a[0], 0x193F22u);
    } else {
        wr32(m, r->a[0], 0x193F30u);
    }
    return 0;
}
