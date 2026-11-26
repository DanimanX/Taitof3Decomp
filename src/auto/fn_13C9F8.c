/* ROM 0x13C9F8: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_13C9F8(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13C9F8 */
    uint32_t sp = r->a[7];
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    Obj_Free(m, r->a[0]);
    r->a[7] = sp;
    return 0;
}
