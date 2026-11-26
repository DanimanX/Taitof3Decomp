/* ROM 0x138382: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_138382(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x138382: Push A0 onto stack */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    /* ROM 0x138384: Call Obj_Free */
    Obj_Free(m, r->a[0]);
    /* ROM 0x13838A: Restore stack pointer */
    r->a[7] += 4;
    /* ROM 0x13838E: Return */
    return 0;
}
