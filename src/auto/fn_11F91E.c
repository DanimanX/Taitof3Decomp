/* ROM 0x11F91E: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11F91E(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F91E: subq.w #1, (0x6,A0) */
    uint16_t val = rd16(m, r->a[0] + 0x6u);
    val--;
    wr16(m, r->a[0] + 0x6u, val);
    if (val > 0) {
        /* ROM 0x11F916: move.l #0x11f91e, (A0) */
        wr32(m, r->a[0], 0x0011F91Eu);
        return 0;
    }

    /* ROM 0x11F924: addq.b #1, (0x3262,A5) */
    uint8_t bval = rd8(m, F3_A5 + 0x3262u);
    bval++;
    wr8(m, F3_A5 + 0x3262u, bval);

    /* ROM 0x11F928: cmpi.b #4, (0x3262,A5) */
    if (bval < 4) {
        /* ROM 0x11F90E: move.w #0xa, (0x6,A0) */
        wr16(m, r->a[0] + 0x6u, 0x000Au);
        /* ROM 0x11F914: beq.b 0x11F924 (never taken) */
        /* ROM 0x11F916: move.l #0x11f91e, (A0) */
        wr32(m, r->a[0], 0x0011F91Eu);
        return 0;
    }

    /* ROM 0x11F930: sf (0x3a,A0) */
    wr8(m, r->a[0] + 0x3Au, 0xFFu);
    /* ROM 0x11F934: clr.w (0x3a,A0) */
    wr16(m, r->a[0] + 0x3Au, 0u);

    /* ROM 0x11F938: bfset (0x30ee,A5){0x7:0x1} */
    uint8_t bf = rd8(m, F3_A5 + 0x30EEu);
    wr8(m, F3_A5 + 0x30EEu, bf | 0x80u);

    /* ROM 0x11F93E: move.w #0xf0, (0x6,A0) */
    wr16(m, r->a[0] + 0x6u, 0x00F0u);

    /* ROM 0x11F944: move.l #0x11f94c, (A0) */
    wr32(m, r->a[0], 0x0011F94Cu);
    return 0;
}
