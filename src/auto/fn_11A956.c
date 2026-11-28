/* ROM 0x11A956: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11A956(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11A956: move.w #0x14,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x14u);
    /* ROM 0x11A95C: beq.b 0x11a96c (never taken after move.w) */
    /* ROM 0x11A95E: move.l #0x11a966,(A0) */
    wr32(m, r->a[0], 0x11A966u);
    /* ROM 0x11A964: rts */
    return 0;
}
