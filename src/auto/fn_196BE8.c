/* ROM 0x196BE8: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_196BE8(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x196BE8: Decrement word at A0+6, branch to 0x196BE0 if result > 0 */
    uint16_t tmp = rd16(m, r->a[0] + 6);
    tmp--;
    wr16(m, r->a[0] + 6, tmp);
    if ((int16_t)tmp > 0) {
        /* ROM 0x196BE0: Store 0x196BE8 to (A0) and return */
        wr32(m, r->a[0], 0x00196BE8u);
        return 0;
    }

    /* ROM 0x196BEE: Falls through */
    r->a[3] = rd32(m, r->a[0] + 0x62);
    wr32(m, r->a[3] + 0x24, 0u);
    wr32(m, r->a[3] + 0x36, rd32(m, r->a[0] + 0x36));
    wr16(m, r->a[3] + 0x34, rd16(m, r->a[0] + 0x34));

    /* ROM 0x196C02: Store 0x196C0A to (A0) and return */
    wr32(m, r->a[0], 0x00196C0Au);
    return 0;
}
