/* ROM 0x0CEF00: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_0CEF00(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0CEF00: move.l A0,-(SP) */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    /* ROM 0x0CEF02: jsr 0x000105ec.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x0CEF08);
    Obj_Free(m, r->a[0]);
    /* ROM 0x0CEF08: lea (0x4,SP),SP */
    r->a[7] += 4;
    /* ROM 0x0CEF0C: rts */
    return 0;
}
