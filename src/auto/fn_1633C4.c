/* ROM 0x1633C4: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_1633C4(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1633C4: Store return address 0x1633CC into (A0) */
    wr32(m, r->a[0], 0x1633CCu);
    return 0;
}
