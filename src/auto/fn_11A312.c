/* ROM 0x11A312: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11A312(const f3mem_t *m, f3regs_t *r) {
    (void)r;
    /* ROM 0x11A312: Move longword from A0+0xC to A0+0x8 */
    wr32(m, r->a[0] + 0x8, rd32(m, r->a[0] + 0xC));
    
    /* ROM 0x11A318: Store 0x11A320 at A0 */
    wr32(m, r->a[0], 0x11A320);
    
    /* ROM 0x11A31E: RTS */
    return 0;
}
