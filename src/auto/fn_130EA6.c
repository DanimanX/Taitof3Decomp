/* ROM 0x130EA6: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

/* ROM 0x130EA6: Push A0, call Obj_Free, restore SP, return */
int Fn_130EA6(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x130EA6: move.l A0,-(SP) */
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[0]);

    /* ROM 0x130EA8: jsr 0x000105ec.l */
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x130EAEu);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4u;

    /* ROM 0x130EAE: lea (0x4,SP),SP */
    r->a[7] += 4u;

    /* ROM 0x130EB2: rts */
    return 0;
}
