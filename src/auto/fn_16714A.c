/* ROM 0x16714A: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_16714A(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x16714A: Subtract 0x240 from long at A5+0x4240 */
    wr32(m, F3_A5 + 0x4240u, rd32(m, F3_A5 + 0x4240u) - 0x240u);
    /* ROM 0x167152: Subtract 0x1A0 from long at A5+0x4230 */
    wr32(m, F3_A5 + 0x4230u, rd32(m, F3_A5 + 0x4230u) - 0x1A0u);
    /* ROM 0x16715A: Decrement word at A0+0x44 */
    uint16_t val = rd16(m, r->a[0] + 0x44u);
    uint16_t new_val = val - 1u;
    wr16(m, r->a[0] + 0x44u, new_val);
    /* ROM 0x16715E: Branch to 0x167142 if signed >= 0 */
    if ((int16_t)new_val >= 0) {
        /* ROM 0x167142: Store 0x16714A to (A0) and RTS */
        wr32(m, r->a[0], (uint32_t)0x16714Au);
        return 0;
    }
    /* ROM 0x167160: Store 0x7F to word at A0+0x44 */
    wr16(m, r->a[0] + 0x44u, 0x7Fu);
    /* ROM 0x167166: Store 0x16716E to (A0) */
    wr32(m, r->a[0], (uint32_t)0x16716Eu);
    /* ROM 0x16716C: RTS */
    return 0;
}
