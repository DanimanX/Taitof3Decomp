/* ROM 0x11CCAA: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11CCAA(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11CCAA: subq.w #1, (6,A0) */
    int16_t tmp = (int16_t)rd16(m, r->a[0] + 6u);
    tmp--;
    wr16(m, r->a[0] + 6u, (uint16_t)tmp);
    
    /* ROM 0x11CCAE: bgt.b 0x11CCA2 (branch if result > 0) */
    if (tmp > 0) {
        /* ROM 0x11CCA2: move.l #0x11CCAA, (A0) */
        wr32(m, r->a[0], 0x11CCAAu);
    } else {
        /* ROM 0x11CCB0: move.l #0x11CCB8, (A0) */
        wr32(m, r->a[0], 0x11CCB8u);
    }
    
    /* ROM 0x11CCA8 / 0x11CCB6: rts */
    return 0;
}
