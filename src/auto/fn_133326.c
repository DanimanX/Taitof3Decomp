/* ROM 0x133326: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133326(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133326: subq.w #1, (0x6,A0) */
    uint16_t cnt = rd16(m, r->a[0] + 6u);
    cnt--;
    wr16(m, r->a[0] + 6u, cnt);
    if (cnt > 0) {
        /* ROM 0x13331E: move.l #0x133326,(A0) ; rts */
        wr32(m, r->a[0], 0x00133326u);
        return 0;
    }

    /* ROM 0x13332C: bfset (0x30ee,A5){0x9:0x1} */
    uint8_t v = rd8(m, F3_A5 + 0x30EEu);
    v |= (1u << (7 - (9 & 7)));
    wr8(m, F3_A5 + 0x30EEu, v);

    /* ROM 0x133332: move.b #0,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 0u);

    /* ROM 0x133338: bfset (0x30ee,A5){0xc:0x1} */
    v = rd8(m, F3_A5 + 0x30EEu);
    v |= (1u << (7 - (0xC & 7)));
    wr8(m, F3_A5 + 0x30EEu, v);

    /* ROM 0x13333E: move.w #0x168,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x0168u);

    /* ROM 0x133344: beq.b 0x133354 (never taken, 0x168 != 0) */
    /* Falls through to 0x133346 */

    /* ROM 0x133346: move.l #0x13334e,(A0) */
    wr32(m, r->a[0], 0x0013334Eu);

    /* ROM 0x13334C: rts */
    return 0;
}
