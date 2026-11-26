/* ROM 0x19A1A0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

/* ROM 0x0105EC: Free object */
void Obj_Free(const f3mem_t *m, uint32_t obj);

int Fn_19A1A0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x19A1A0: move.w #0xa,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x0Au);
    
    /* ROM 0x19A1A6: beq.b 0x19A1B6 (move.w does not affect flags; folded as never taken per hint) */
    {
        /* ROM 0x19A1A8: move.l #0x19a1b0,(A0) */
        wr32(m, r->a[0], 0x19A1B0u);
    }
    return 0;
}
