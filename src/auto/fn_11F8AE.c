/* ROM 0x11F8AE: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11F8AE(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F8AE: Decrement word at A0+6 */
    uint16_t val = rd16(m, r->a[0] + 6u);
    val--;
    wr16(m, r->a[0] + 6u, val);

    /* ROM 0x11F8B2: bgt.b 0x11F8A6 (branch if decremented value > 0) */
    if ((int16_t)val > 0) {
        /* ROM 0x11F8A6: move.l #0x11f8ae,(A0) ; rts */
        wr32(m, r->a[0], 0x0011F8AEu);
        return 0;
    }

    /* ROM 0x11F8B4: falls through */
    /* bfset (0x30ee,A5){0x1:0x1} -> set bit 6 (offset 1 from MSB) of byte at A5+0x30EE */
    uint32_t bf_addr = F3_A5 + 0x30EEu;
    uint8_t bf_byte = rd8(m, bf_addr);
    bf_byte |= (uint8_t)(1u << (7 - 1));
    wr8(m, bf_addr, bf_byte);

    /* ROM 0x11F8BA: move.w #0xa,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x000Au);

    /* ROM 0x11F8C0: beq.b 0x0011f8d0 (folded: never taken after setting non-zero constant) */

    /* ROM 0x11F8C2: move.l #0x11f8ca,(A0) */
    wr32(m, r->a[0], 0x0011F8CAu);

    /* ROM 0x11F8C8: rts */
    return 0;
}
