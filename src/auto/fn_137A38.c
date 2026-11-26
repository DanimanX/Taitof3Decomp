/* ROM 0x137A38: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137A38(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137A38: Decrement word at A0+6, branch to 0x137A30 if > 0 */
    uint16_t tmp = rd16(m, r->a[0] + 6);
    tmp--;
    wr16(m, r->a[0] + 6, tmp);
    if ((int16_t)tmp > 0) {
        /* ROM 0x137A30: Store 0x137A38 to A0 and return */
        wr32(m, r->a[0], 0x137A38u);
        return 0;
    }
    /* ROM 0x137A3E: Store 0x400 to A0+0x42 */
    wr16(m, r->a[0] + 0x42, 0x400u);
    /* ROM 0x137A44: Store 0x137A4C to A0 and return */
    wr32(m, r->a[0], 0x137A4Cu);
    return 0;
}
