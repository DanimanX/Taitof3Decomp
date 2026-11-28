/* ROM 0x11E252: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x11E252: Push A0, call Obj_Free, restore SP, return */
int Fn_11E252(const f3mem_t *m, f3regs_t *r) {
    uint32_t saved_a0 = r->a[0];
    r->a[7] -= 4;
    wr32(m, r->a[7], saved_a0);
    Obj_Free(m, saved_a0);
    r->a[7] += 4;
    return 0;
}
