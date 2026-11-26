/* ROM 0x121AEE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_121AEE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x121AEE: Subtract 0x6000 from long word at A5+0x30D6 */
    uint32_t v = rd32(m, F3_A5 + 0x30D6u);
    v = v - 0x6000u;
    wr32(m, F3_A5 + 0x30D6u, v);

    /* ROM 0x121AF6: Compare word at A5+0x30D6 with 0x78 */
    uint16_t w = rd16(m, F3_A5 + 0x30D6u);
    /* ROM 0x121AFC: Branch if greater (signed) to 0x121AE6 */
    if ((int16_t)w > 0x78) {
        /* LAB_00121ae6: move.l #0x121aee,(A0) ; rts */
        wr32(m, r->a[0], 0x121AEEu);
        return 0;
    }

    /* ROM 0x121AFE: bfset (0x30ee,A5){0xc:0x1} */
    /* On memory, offset 0xC mod 8 = 4. From MSB, this is bit 4 (mask 0x10). */
    uint8_t b = rd8(m, F3_A5 + 0x30EEu);
    b |= 0x10u;
    wr8(m, F3_A5 + 0x30EEu, b);

    /* ROM 0x121B04: move.b #0x77,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 0x77u);

    /* Falls through to LAB_00121b0a */
    /* ROM 0x121B0A: move.l #0x121b12,(A0) */
    wr32(m, r->a[0], 0x121B12u);
    /* ROM 0x121B10: rts */
    return 0;
}
