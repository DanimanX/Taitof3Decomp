/* ROM 0x137996: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_137996(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137996: cmpi.w #4, (0x32d6,A5) */
    if ((int16_t)rd16(m, F3_A5 + 0x32d6u) >= 4) {
        /* ROM 0x137B2C: branch target */
        wr16(m, F3_A5 + 0x3318u, 0u);
        wr8(m, r->a[0] + 0x54u, 2u);
        wr16(m, r->a[0] + 0x50u, 0x1d0u);
        wr16(m, r->a[0] + 0x52u, 0x78u);
        wr16(m, r->a[0] + 0x6u, 0x12cu);
        /* ROM 0x137B48: beq.b 0x137b6a (never taken, falls through) */
        /* ROM 0x137B4A: move.l #0x137b52,(A0) */
        wr32(m, r->a[0], 0x00137b52u);
        return 0;
    }

    /* ROM 0x1379A0: subq.w #1, (0x32d0,A5) */
    wr16(m, F3_A5 + 0x32d0u, (uint16_t)(rd16(m, F3_A5 + 0x32d0u) - 1u));

    /* ROM 0x1379A4: addq.w #1, (0x32d4,A5) */
    wr16(m, F3_A5 + 0x32d4u, (uint16_t)(rd16(m, F3_A5 + 0x32d4u) + 1u));

    /* ROM 0x1379A8: subq.w #1, (6,A0) */
    {
        uint16_t val = (uint16_t)(rd16(m, r->a[0] + 0x6u) - 1u);
        wr16(m, r->a[0] + 0x6u, val);
        /* ROM 0x1379AC: bgt.b 0x13798e */
        if ((int16_t)val > 0) {
            /* ROM 0x13798E: move.l #0x137996,(A0) */
            wr32(m, r->a[0], 0x00137996u);
            /* ROM 0x137994: rts */
            return 0;
        }
    }

    /* ROM 0x1379AE: bfset (0x30ee,A5){0xc:0x1} */
    {
        uint32_t v = rd32(m, F3_A5 + 0x30eeu);
        v |= (1u << (31 - 0xc));
        wr32(m, F3_A5 + 0x30eeu, v);
    }

    /* ROM 0x1379B4: move.l (0x1d28,A5),(0x30e2,A5) */
    wr32(m, F3_A5 + 0x30e2u, rd32(m, F3_A5 + 0x1d28u));

    /* ROM 0x1379BA: move.w #0xf0,(6,A0) */
    wr16(m, r->a[0] + 0x6u, 0xf0u);

    /* ROM 0x1379C0: clr.w (0x42,A0) */
    wr16(m, r->a[0] + 0x42u, 0u);

    /* ROM 0x1379C4: move.l #0x1379cc,(A0) */
    wr32(m, r->a[0], 0x001379ccu);
    /* ROM 0x1379CA: rts */
    return 0;
}
