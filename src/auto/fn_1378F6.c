/* ROM 0x1378F6: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_1378F6(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1378F6: Decrement word at A0+6, branch if >0 to 0x1378EE */
    uint16_t val = rd16(m, r->a[0] + 0x6u);
    val--;
    wr16(m, r->a[0] + 0x6u, val);
    if ((int16_t)val > 0) {
        /* LAB_001378EE: Store return address and RTS */
        wr32(m, r->a[0], 0x1378F6u);
        return 0;
    }

    /* LAB_001378FC: Initialize state */
    wr8(m, r->a[0] + 0x54u, 2u);
    wr16(m, r->a[0] + 0x50u, 0x260u);
    wr16(m, r->a[0] + 0x52u, 0x78u);
    wr16(m, r->a[0] + 0x6u, 0x12cu);

    /* beq.b 0x137936 (folded: move.w does not affect flags, assumed never taken) */
    if (0) {
        /* LAB_00137936: Set bit 10 of A5+0x30EE, set state to 0x3C, RTS */
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        wr8(m, F3_A5 + 0x30EEu, b | (1 << 2));
        wr16(m, r->a[0] + 0x6u, 0x3cu);
        wr32(m, r->a[0], 0x13794Cu);
        return 0;
    }

    /* LAB_00137916: RTS */
    wr32(m, r->a[0], 0x13791Eu);
    return 0;

    /* beq.b 0x137952 (folded) */
    if (0) {
        /* LAB_00137952: Set bit 11 of A5+0x30EE, set state to 0x1E, RTS */
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        wr8(m, F3_A5 + 0x30EEu, b | (1 << 3));
        wr16(m, r->a[0] + 0x6u, 0x1eu);
        wr32(m, r->a[0], 0x137968u);
        return 0;
    }

    /* LAB_00137960: RTS */
    wr32(m, r->a[0], 0x137968u);
    return 0;

    /* beq.b 0x13796E (folded) */
    if (0) {
        /* LAB_0013796E: Clear bit 10 of A5+0x30EE, call sound routines, RTS */
        uint8_t b = rd8(m, F3_A5 + 0x30EEu);
        wr8(m, F3_A5 + 0x30EEu, b & ~(1 << 2));
        r->a[7] -= 2;
        wr16(m, r->a[7], 0x3au);

        r->a[7] -= 4;
        wr32(m, r->a[7], 0x13797Eu);
        Sound_PlayHooked(m, (uint16_t)rd16(m, r->a[7] + 4));
        r->a[7] += 4;

        r->a[7] -= 4;
        wr32(m, r->a[7], 0x137984u);
        /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
        r->a[7] += 4;

        r->a[7] += 2;
        wr16(m, r->a[0] + 0x6u, 0x3cu);
        wr32(m, r->a[0], 0x137996u);
        return 0;
    }

    /* LAB_0013798E: RTS */
    wr32(m, r->a[0], 0x137996u);
    return 0;
}
