/* ROM 0x13A796: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_13A796(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13A796: move.l A0,-(SP) */
    /* ROM 0x13A798: jsr 0x000105ec.l */
    Obj_Free(m, r->a[0]);
    /* ROM 0x13A79E: lea (0x4,SP),SP */
    r->a[7] += 4;
    /* ROM 0x13A7A2: rts */
    return 0;
}
