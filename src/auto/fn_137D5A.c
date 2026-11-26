/* ROM 0x137D5A: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137D5A(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137D5A: Decrement word at A0+6, branch to 0x137D52 if > 0 */
    uint16_t val = (uint16_t)rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);
    if ((int16_t)val > 0) {
        /* ROM 0x137D52: Store 0x137D5A at A0, return */
        wr32(m, r->a[0], 0x137D5Au);
        return 0;
    }

    /* ROM 0x137D60: Clear bit 13 of byte at A5+0x30EE (bit 2 from LSB) */
    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b &= (uint8_t)~0x04u;
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x137D66: Store 0xB4 at A0+6 */
    wr16(m, r->a[0] + 6u, 0x00B4u);

    /* ROM 0x137D6C: Branch to 0x137D8E (beq folded per hint) */
    /* ROM 0x137D8E: Set bit 14 of byte at A5+0x30EE (bit 1 from LSB) */
    b = rd8(m, F3_A5 + 0x30EEu);
    b |= (uint8_t)0x02u;
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x137D94: Store 0x1E0 at A0+6 */
    wr16(m, r->a[0] + 6u, 0x01E0u);

    /* ROM 0x137D9A: Branch to 0x137DBC (beq folded per hint) */
    /* ROM 0x137DBC: Clear bit 14 of byte at A5+0x30EE */
    b = rd8(m, F3_A5 + 0x30EEu);
    b &= (uint8_t)~0x02u;
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x137DC2: Branch to 0x137D30 */
    /* ROM 0x137D30: Set bit 13 of byte at A5+0x30EE */
    b = rd8(m, F3_A5 + 0x30EEu);
    b |= (uint8_t)0x04u;
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x137D36: Store 0x137D3E at A0 */
    wr32(m, r->a[0], 0x137D3Eu);

    /* ROM 0x137D3C: RTS */
    return 0;
}
