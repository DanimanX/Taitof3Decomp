/* ROM 0x181556: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_181556(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x181556: Decrement word at A0+6, branch if > 0 to 0x18154E */
    uint16_t val = rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);
    if ((int16_t)val > 0) {
        /* ROM 0x18154E: Set next routine to 0x181556 */
        wr32(m, r->a[0], 0x181556u);
    } else {
        /* ROM 0x18155C: Branch to 0x17BE14 -> Set next routine to 0x17BE1C */
        wr32(m, r->a[0], 0x17be1cu);
    }
    return 0;
}
