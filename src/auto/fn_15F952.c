/* ROM 0x15F952: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_15F952(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x15F952: Store address 0x15F95A to memory at A0 */
    wr32(m, r->a[0], 0x15F95Au);
    return 0;
}
