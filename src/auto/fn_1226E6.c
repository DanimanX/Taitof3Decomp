/* ROM 0x1226E6: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

/* ROM 0x1226E6: Increment long at A5+0x30D2 by 0x40000, decrement word at A0+0x22.
 * If the decremented word is > 0 (signed), store 0x1226E6 at A0 and return.
 * Otherwise store 0x78 at A0+0x22, store 0x122702 at A0, and return. */
int Fn_1226E6(const f3mem_t *m, f3regs_t *r) {
    /* 1226E6: addi.l #0x40000, (0x30d2, A5) */
    wr32(m, F3_A5 + 0x30d2u, rd32(m, F3_A5 + 0x30d2u) + 0x40000u);

    /* 1226EE: subq.w #1, (0x22, A0) */
    uint16_t val = (uint16_t)(rd16(m, r->a[0] + 0x22u) - 1u);
    wr16(m, r->a[0] + 0x22u, val);

    /* 1226F2: bgt.b LAB_1226DE */
    if ((int16_t)val > 0) {
        /* 1226DE: move.l #0x1226E6, (A0) */
        wr32(m, r->a[0], 0x1226E6u);
        /* 1226E4: rts */
        return 0;
    }

    /* 1226F4: move.w #0x78, (0x22, A0) */
    wr16(m, r->a[0] + 0x22u, 0x78u);

    /* 1226FA: move.l #0x122702, (A0) */
    wr32(m, r->a[0], 0x122702u);

    /* 122700: rts */
    return 0;
}
