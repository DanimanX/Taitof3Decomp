/* ROM 0x137542: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137542(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137542: addi.w #0xa,(0x30d2,A5) */
    uint16_t v = rd16(m, F3_A5 + 0x30d2u);
    v += 0xa;
    wr16(m, F3_A5 + 0x30d2u, v);

    /* ROM 0x137548: cmpi.w #0x410,(0x30d2,A5) */
    /* ROM 0x13754E: blt.b 0x0013753a */
    if ((int16_t)v < 0x410) {
        /* ROM 0x13753A: move.l #0x137542,(A0) */
        wr32(m, r->a[0], 0x137542u);
    } else {
        /* ROM 0x137550: move.l #0x137558,(A0) */
        wr32(m, r->a[0], 0x137558u);
    }
    /* ROM 0x137556: rts */
    return 0;
}
