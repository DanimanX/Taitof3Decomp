/* ROM 0x19A1B0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19A1B0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x19A1B0: Decrement word at A0+6, branch to 0x19A1A8 if > 0 */
    uint16_t val = rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);
    if ((int16_t)val > 0) {
        /* ROM 0x19A1A8: Store 0x19A1B0 into A0, return */
        wr32(m, r->a[0], 0x19A1B0u);
        return 0;
    }

    /* ROM 0x19A1B6: Push A0, call Obj_Free, pop A0, return */
    r->a[7] -= 4u;
    wr32(m, r->a[7], r->a[0]);
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x19A1BEu);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4u;
    r->a[7] += 4u;
    return 0;
}
