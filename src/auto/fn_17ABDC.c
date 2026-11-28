/* ROM 0x17ABDC: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_17ABDC(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x17ABDC: Load long from A0+0x0C into D0 */
    r->d[0] = rd32(m, r->a[0] + 0x0Cu);
    /* ROM 0x17ABE0: Store low word of D0 to A0+0x42 */
    wr16(m, r->a[0] + 0x42u, (uint16_t)r->d[0]);
    /* ROM 0x17ABE4: Load immediate long into A1 */
    r->a[1] = 0x17AC56u;
    /* ROM 0x17ABEA: Store A1 to A0+0x44 */
    wr32(m, r->a[0] + 0x44u, r->a[1]);
    /* ROM 0x17ABEE: Load word from A1+0x04, store to A0+0x48 */
    wr16(m, r->a[0] + 0x48u, rd16(m, r->a[1] + 0x04u));
    /* ROM 0x17ABF4: Clear word at A0+0x4A */
    wr16(m, r->a[0] + 0x4Au, 0u);
    /* ROM 0x17ABF8: Store immediate long to A0 */
    wr32(m, r->a[0], 0x17AC00u);
    /* ROM 0x17ABFE: Return */
    return 0;
}
