/* ROM 0x0C8B48: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_0C8B48(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0C8B48: Load word from A1+0x4C into D0, clamp to max 10, use as index into A0 table */
    r->d[0] = (int16_t)rd16(m, r->a[1] + 0x4Cu);
    if ((uint16_t)r->d[0] > 10u) {
        r->d[0] = 10u;
    }
    r->d[0] = rd32(m, r->a[0] + (uint32_t)(int16_t)r->d[0] * 4u);
    return 0;
}
