/* ROM 0x1784F4: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_1784F4(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1784F4: move.w #0xf, D0.w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | 0x000Fu;
    /* ROM 0x1784F8: bsr.w 0x0017855a */
    /* ROM 0x1784FC: dbf D0.w, 0x001784f8 */
    for (;;) {
        (void)Fn_17855A(m, r);
        uint16_t d0w = (uint16_t)r->d[0] - 1u;
        r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
        if (d0w == 0xFFFFu) break;
    }
    /* ROM 0x178500: rts */
    return 0;
}
