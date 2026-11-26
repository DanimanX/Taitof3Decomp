/* ROM 0x1332BA: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_1332BA(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1332BA: Decrement word at A0+6, branch if >0 to 0x1332B2 */
    uint16_t tmp = rd16(m, r->a[0] + 6u);
    tmp--;
    wr16(m, r->a[0] + 6u, tmp);
    if ((int16_t)tmp > 0) {
        /* ROM 0x1332B2: Store 0x1332BA to A0, RTS */
        wr32(m, r->a[0], 0x1332BAu);
        return 0;
    }
    /* ROM 0x1332C0: bfclr bit 11 of word at A5+0x30EE */
    uint16_t v = rd16(m, F3_A5 + 0x30EEu);
    v &= (uint16_t)~(1U << (15 - 0xB));
    wr16(m, F3_A5 + 0x30EEu, v);
    /* ROM 0x1332C6: move.b #7 to A5+0x30F2 */
    wr8(m, F3_A5 + 0x30F2u, 0x7u);
    /* ROM 0x1332CC: bfset bit 12 of word at A5+0x30EE */
    v = rd16(m, F3_A5 + 0x30EEu);
    v |= (uint16_t)(1U << (15 - 0xC));
    wr16(m, F3_A5 + 0x30EEu, v);
    /* ROM 0x1332D2: Store 0x1332DA to A0, RTS */
    wr32(m, r->a[0], 0x1332DAu);
    return 0;
}
