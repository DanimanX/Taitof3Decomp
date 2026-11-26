/* ROM 0x0D7C66: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_0D7C66(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0D7C66: Push A0, call Obj_Free, restore SP, return */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4;
    return 0;
}
