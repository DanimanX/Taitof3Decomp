/* ROM 0x133240: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_133240(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x133240: subq.w #0x1,(0x6,A0) */
    uint16_t cnt = rd16(m, r->a[0] + 6);
    cnt--;
    wr16(m, r->a[0] + 6, cnt);

    /* ROM 0x133244: bgt.b 0x133238 */
    if (cnt > 0) {
        /* ROM 0x133238: move.l #0x133240,(A0) ; rts */
        wr32(m, r->a[0], 0x133240);
        return 0;
    }

    /* ROM 0x133246: bfset (0x30ee,A5){0x1:0x1} */
    uint8_t *p30ee = (uint8_t *)(F3_A5 + 0x30ee);
    *p30ee |= (uint8_t)(1 << (1 % 8));

    /* ROM 0x13324C: bfset (0x30ee,A5){0x6:0x1} */
    *p30ee |= (uint8_t)(1 << (6 % 8));

    /* ROM 0x133252: move.w #0x168,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x168);

    /* ROM 0x133258: beq.b 0x133268 (Z flag from subq.w at 0x133240) */
    if (cnt == 0) {
        /* ROM 0x133268: bfclr (0x30ee,A5){0x6:0x1} */
        *p30ee &= (uint8_t)~(1 << (6 % 8));

        /* ROM 0x13326E: bfclr (0x30ee,A5){0x4:0x1} */
        *p30ee &= (uint8_t)~(1 << (4 % 8));

        /* ROM 0x133274: bfclr (0x30ee,A5){0xb:0x1} */
        *p30ee &= (uint8_t)~(1 << (11 % 8));

        /* ROM 0x13327A: move.b #0x7,(0x30f2,A5) */
        wr8(m, F3_A5 + 0x30f2, 7);

        /* ROM 0x133280: bfset (0x30ee,A5){0xc:0x1} */
        *p30ee |= (uint8_t)(1 << (12 % 8));

        /* ROM 0x133286: move.l #0x13328e,(A0) */
        wr32(m, r->a[0], 0x13328e);
    } else {
        /* ROM 0x13325A: move.l #0x133262,(A0) */
        wr32(m, r->a[0], 0x133262);
    }

    /* ROM 0x133260 / 0x13328C: rts */
    return 0;
}
