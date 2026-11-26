/* ROM 0x0D89F2: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D89F2: Push A0, call Obj_Free, restore SP, return */
int Fn_0D89F2(const f3mem_t *m, f3regs_t *r) {
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x0D89FA);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4;
    r->a[7] += 4;
    return 0;
}
