/* ROM 0x0CEEF0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_0CEEF0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0CEEF0: Store 0x0CEEF8 to address in A0 */
    wr32(m, r->a[0], 0x0CEEF8u);
    return 0;
}
