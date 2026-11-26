/* ROM 0x13B8A6: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_13B8A6(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13B8A6: Push A0, call Obj_Free, restore SP, return */
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[0]);
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x13B8AEu);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4u;
    r->a[7] += 4u;
    return 0;
}
