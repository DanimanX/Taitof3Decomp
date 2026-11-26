/* ROM 0x137C8A: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137C8A(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137C8A: subq.w #0x1, (0x6, A0) */
    int16_t cnt = (int16_t)rd16(m, r->a[0] + 6u);
    cnt--;
    wr16(m, r->a[0] + 6u, (uint16_t)cnt);

    /* ROM 0x137C8E: bgt.b 0x137C82 */
    if (cnt > 0) {
        /* ROM 0x137C82: move.l #0x137c8a, (A0) */
        wr32(m, r->a[0], 0x00137C8Au);
        /* ROM 0x137C88: rts */
        return 0;
    }

    /* ROM 0x137C90: move.b #0x2, (0x54, A0) */
    wr8(m, r->a[0] + 0x54u, 0x02u);

    /* ROM 0x137C96: move.w #0x9f, (0x6, A0) */
    wr16(m, r->a[0] + 0x06u, 0x009Fu);

    /* ROM 0x137C9C: move.l #0x137ca4, (A0) */
    wr32(m, r->a[0], 0x00137CA4u);

    /* ROM 0x137CA2: rts */
    return 0;
}
