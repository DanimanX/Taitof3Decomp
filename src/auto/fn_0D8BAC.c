/* ROM 0x0D8BAC: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D8BAC: Push A0, call Obj_Free, clean stack, return */
int Fn_0D8BAC(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0D8BAC: move.l A0,-(SP) */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    /* ROM 0x0D8BAE: jsr 0x000105ec.l */
    Obj_Free(m, r->a[0]);
    /* ROM 0x0D8BB4: lea (0x4,SP),SP */
    r->a[7] += 4;
    /* ROM 0x0D8BB8: rts */
    return 0;
}
