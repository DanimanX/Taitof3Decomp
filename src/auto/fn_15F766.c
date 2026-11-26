/* ROM 0x15F766: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_15F766(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x15F766: Store 0x15F76E to address in A0 */
    wr32(m, r->a[0], 0x15F76Eu);
    return 0;
}
