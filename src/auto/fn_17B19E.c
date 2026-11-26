/* ROM 0x17B19E: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_17B19E(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x17B19E: Store 0x17B1A6 to address in A0 */
    wr32(m, r->a[0], 0x17B1A6u);
    return 0;
}
