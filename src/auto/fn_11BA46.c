/* ROM 0x11BA46: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11BA46(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11BA46: Move long from A5+0x1D28 to A0+0xC */
    uint32_t val = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, r->a[0] + 0xCu, val);

    /* ROM 0x11BA4C: Branch to 0x11BA3E if zero */
    if (val == 0) {
        /* ROM 0x11BA3E: Store 0x11BA46 to A0 */
        wr32(m, r->a[0], 0x11BA46u);
        return 0;
    }

    /* ROM 0x11BA4E: Store 0x1E to A0+0x6 */
    wr16(m, r->a[0] + 0x6u, 0x1Eu);

    /* ROM 0x11BA54: Store 0x11BA5C to A0 */
    wr32(m, r->a[0], 0x11BA5Cu);

    /* ROM 0x11BA5A: RTS */
    return 0;
}
