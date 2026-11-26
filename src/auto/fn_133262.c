/* ROM 0x133262: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133262(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133262: Decrement word at A0+6, branch to 0x13325A if > 0 */
    uint16_t val = (uint16_t)(rd16(m, r->a[0] + 6u) - 1u);
    wr16(m, r->a[0] + 6u, val);
    if ((int16_t)val > 0) {
        /* ROM 0x13325A: Store 0x133262 to (A0) and return */
        wr32(m, r->a[0], 0x00133262u);
        return 0;
    }

    /* ROM 0x133268: bfclr bit 6 (byte bit 1) at A5+0x30EE */
    uint8_t v = rd8(m, F3_A5 + 0x30EEu);
    v &= ~0x02u; wr8(m, F3_A5 + 0x30EEu, v);

    /* ROM 0x13326E: bfclr bit 4 (byte bit 3) at A5+0x30EE */
    v = rd8(m, F3_A5 + 0x30EEu);
    v &= ~0x08u; wr8(m, F3_A5 + 0x30EEu, v);

    /* ROM 0x133274: bfclr bit 0xB (byte bit 4) at A5+0x30EE */
    v = rd8(m, F3_A5 + 0x30EEu);
    v &= ~0x10u; wr8(m, F3_A5 + 0x30EEu, v);

    /* ROM 0x13327A: move.b #7 to (A5+0x30F2) */
    wr8(m, F3_A5 + 0x30F2u, 0x07u);

    /* ROM 0x133280: bfset bit 0xC (byte bit 3) at A5+0x30EE */
    v = rd8(m, F3_A5 + 0x30EEu);
    v |= 0x08u; wr8(m, F3_A5 + 0x30EEu, v);

    /* ROM 0x133286: Store 0x13328E to (A0) and return */
    wr32(m, r->a[0], 0x0013328Eu);
    return 0;
}
