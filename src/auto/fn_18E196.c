/* ROM 0x18E196: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_18E196(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x18E196: Decrement word at A0+6, branch to 0x18E18E if > 0 */
    uint16_t val = rd16(m, r->a[0] + 6);
    val--;
    wr16(m, r->a[0] + 6, val);
    if ((int16_t)val > 0) {
        /* ROM 0x18E18E: Store return address at A0 and return */
        wr32(m, r->a[0], (uint32_t)0x18E196);
        return 0;
    }
    /* ROM 0x18E19C: Push A0, call Obj_Free, pop A0, return */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4;
    return 0;
}
