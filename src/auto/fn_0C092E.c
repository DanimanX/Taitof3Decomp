/* ROM 0x0C092E: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_0C092E(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x0C092E: Decrement word at A0+6, branch to 0x0C0926 if > 0 */
    uint16_t v = (uint16_t)(rd16(m, r->a[0] + 6u) - 1u);
    wr16(m, r->a[0] + 6u, v);
    if ((int16_t)v > 0) {
        /* ROM 0x0C0926: Store 0xC092E at A0, then RTS */
        wr32(m, r->a[0], 0xC092Eu);
        return 0;
    }
    /* ROM 0x0C0934: Store 0x1E at A0+0x60 */
    wr16(m, r->a[0] + 0x60u, 0x1Eu);
    /* ROM 0x0C093A: Store 0xC0942 at A0, then RTS */
    wr32(m, r->a[0], 0xC0942u);
    return 0;
}
