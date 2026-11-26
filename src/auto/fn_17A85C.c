/* ROM 0x17A85C: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_17A85C(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x17A85C: subq.w #0x1,(0x6,A0) */
    uint16_t val = rd16(m, r->a[0] + 6);
    uint16_t new_val = val - 1;
    wr16(m, r->a[0] + 6, new_val);

    /* ROM 0x17A860: bgt.b 0x0017a854 */
    if ((int16_t)new_val > 0) {
        /* ROM 0x17A854: move.l #0x17a85c,(A0) ; rts */
        wr32(m, r->a[0], (uint32_t)0x0017A85C);
        return 0;
    }

    /* ROM 0x17A862: move.l A0,-(SP) */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);

    /* ROM 0x17A864: jsr 0x000105ec.l */
    Obj_Free(m, r->a[0]);

    /* ROM 0x17A86A: lea (0x4,SP),SP */
    r->a[7] += 4;

    /* ROM 0x17A86E: rts */
    return 0;
}
