/* ROM 0x0CEEF8: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_0CEEF8(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0CEEF8: Store 0x0000CEF00 to address in A0 */
    wr32(m, r->a[0], 0x0000CEF00u);
    return 0;
}
