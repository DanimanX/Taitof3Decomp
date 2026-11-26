/* ROM 0x137A92: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137A92(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137A92: Decrement word at A0+6, branch if >0 to re-arm self */
    uint16_t v = rd16(m, r->a[0] + 6u);
    v--;
    wr16(m, r->a[0] + 6u, v);
    if ((int16_t)v > 0) {
        wr32(m, r->a[0], 0x137A92u);
        return 0;
    }
    /* ROM 0x137A98: Fallthrough path */
    wr16(m, r->a[0] + 6u, 0xB4u);
    wr16(m, r->a[0] + 0x42u, 0u);
    wr32(m, r->a[0], 0x137AAAu);
    return 0;
}
