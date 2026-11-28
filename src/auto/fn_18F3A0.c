/* ROM 0x18F3A0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_18F3A0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x18F3A0: Load long from A0+0xC into D0 */
    r->d[0] = rd32(m, r->a[0] + 0x0Cu);
    /* ROM 0x18F3A4: Store low word of D0 to A0+0x42 */
    wr16(m, r->a[0] + 0x42u, (uint16_t)r->d[0]);
    /* ROM 0x18F3A8: Load immediate into A1 */
    r->a[1] = 0x17AC56u;
    /* ROM 0x18F3AE: Store A1 to A0+0x44 */
    wr32(m, r->a[0] + 0x44u, r->a[1]);
    /* ROM 0x18F3B2: Load word from A1+0x4, store to A0+0x48 */
    wr16(m, r->a[0] + 0x48u, rd16(m, r->a[1] + 0x04u));
    /* ROM 0x18F3B8: Clear word at A0+0x4A */
    wr16(m, r->a[0] + 0x4Au, 0u);
    /* ROM 0x18F3BC: Store immediate to A0 */
    wr32(m, r->a[0], 0x18F3C4u);
    /* ROM 0x18F3C2: Return */
    return 0;
}
