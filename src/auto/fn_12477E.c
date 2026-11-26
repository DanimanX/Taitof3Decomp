/* ROM 0x12477E: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_12477E(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x12477E: Decrement word at A0+6, branch if >0 to 0x124776 */
    uint16_t v = rd16(m, r->a[0] + 6);
    v--;
    wr16(m, r->a[0] + 6, v);
    if (v > 0) {
        /* ROM 0x124776: Store 0x12477E to (A0) and return */
        wr32(m, r->a[0], 0x12477Eu);
    } else {
        /* ROM 0x124784: Store 0x12478C to (A0) and return */
        wr32(m, r->a[0], 0x12478Cu);
    }
    return 0;
}
