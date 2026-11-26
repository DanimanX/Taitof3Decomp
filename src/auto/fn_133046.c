/* ROM 0x133046: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133046(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133046: Decrement word at A0+6 */
    uint16_t v = rd16(m, r->a[0] + 0x6u);
    v--;
    wr16(m, r->a[0] + 0x6u, v);
    
    /* ROM 0x13304A: Branch to 0x13303E if result > 0 (signed) */
    if ((int16_t)v > 0) {
        /* ROM 0x13303E: Store 0x133046 to A0 */
        wr32(m, r->a[0], 0x133046u);
        /* ROM 0x133044: RTS */
        return 0;
    }
    
    /* ROM 0x13304C: Store 0x3C to A0+6 */
    wr16(m, r->a[0] + 0x6u, 0x3Cu);
    
    /* ROM 0x133052: Store 0x13305A to A0 */
    wr32(m, r->a[0], 0x13305Au);
    
    /* ROM 0x133058: RTS */
    return 0;
}
