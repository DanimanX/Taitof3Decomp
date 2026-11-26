/* ROM 0x133396: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133396(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133396: Increment counter at A5+0x30D2 by 0xC000 */
    uint32_t cnt = rd32(m, F3_A5 + 0x30D2u);
    wr32(m, F3_A5 + 0x30D2u, cnt + 0xC000u);

    /* ROM 0x13339E: Compare counter with 0xA0 */
    if (rd16(m, F3_A5 + 0x30D2u) < 0xA0u) {
        /* ROM 0x13338E: Re-arm routine */
        wr32(m, r->a[0], 0x133396u);
        return 0;
    }

    /* ROM 0x1333A6: Set byte at A5+0x30F2 to 0x8 */
    wr8(m, F3_A5 + 0x30F2u, 0x8u);

    /* ROM 0x1333AC: bfset (0x30ee,A5){0xc:0x1} -> offset 12 -> bit 3 */
    {
        uint8_t v = rd8(m, F3_A5 + 0x30EEu);
        v |= (uint8_t)(1u << (7 - (12 & 7)));
        wr8(m, F3_A5 + 0x30EEu, v);
    }

    /* ROM 0x1333B2: Store 0x12C to word at A0+6 */
    wr16(m, r->a[0] + 6u, 0x12cu);

    /* ROM 0x1333B8: beq.b 0x001333c8 (dead code, move.w doesn't set flags) */
    /* ROM 0x1333BA: Fallthrough to store 0x1333C2 to (A0) */
    wr32(m, r->a[0], 0x1333c2u);
    return 0;
}
