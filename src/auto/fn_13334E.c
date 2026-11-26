/* ROM 0x13334E: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_13334E(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13334E: subq.w #1, (6,A0) */
    uint16_t cnt = rd16(m, r->a[0] + 6u);
    cnt--;
    wr16(m, r->a[0] + 6u, cnt);

    /* ROM 0x133352: bgt.b 0x133346 */
    if ((int16_t)cnt > 0) {
        /* ROM 0x133346: move.l #0x13334e, (A0) */
        wr32(m, r->a[0], 0x0013334Eu);
        /* ROM 0x13334C: rts */
        return 0;
    }

    /* ROM 0x133354: bfset (0x30ee,A5){0x1:0x1} */
    uint32_t p30ee = F3_A5 + 0x30EEu;
    uint8_t v = rd8(m, p30ee);
    v |= (uint8_t)(1u << 1u);
    wr8(m, p30ee, v);

    /* ROM 0x13335A: bfset (0x30ee,A5){0x6:0x1} */
    v = rd8(m, p30ee);
    v |= (uint8_t)(1u << 6u);
    wr8(m, p30ee, v);

    /* ROM 0x133360: move.w #0x168, (0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x0168u);

    /* ROM 0x133366: beq.b 0x133376 (Z flag from subq.w) */
    if (cnt == 0) {
        /* ROM 0x133376: bfclr (0x30ee,A5){0x6:0x1} */
        v = rd8(m, p30ee);
        v &= (uint8_t)~(1u << 6u);
        wr8(m, p30ee, v);

        /* ROM 0x13337C: bfset (0x30ee,A5){0xb:0x1} */
        v = rd8(m, p30ee);
        v |= (uint8_t)(1u << 11u);
        wr8(m, p30ee, v);

        /* ROM 0x133382: move.b #0x7, (0x30f2,A5) */
        wr8(m, F3_A5 + 0x30F2u, 0x07u);

        /* ROM 0x133388: bfset (0x30ee,A5){0xc:0x1} */
        v = rd8(m, p30ee);
        v |= (uint8_t)(1u << 12u);
        wr8(m, p30ee, v);

        /* falls through to 0x13338E */
    } else {
        /* ROM 0x133368: move.l #0x133370, (A0) */
        wr32(m, r->a[0], 0x00133370u);
        /* ROM 0x13336E: rts */
        return 0;
    }

    /* ROM 0x13338E: move.l #0x133396, (A0) */
    wr32(m, r->a[0], 0x00133396u);
    /* ROM 0x133394: rts */
    return 0;
}
