/* ROM 0x137968: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../audio/sound_cmd.h"

int Fn_137968(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x137968: subq.w #0x1,(0x6,A0) */
    uint16_t v = rd16(m, r->a[0] + 6);
    v--;
    wr16(m, r->a[0] + 6, v);

    /* ROM 0x13796C: bgt.b 0x00137960 */
    if (v > 0) {
        /* ROM 0x137960: move.l #0x137968,(A0) */
        wr32(m, r->a[0], 0x00137968u);
        return 0;
    }

    /* ROM 0x13796E: bfclr (0x30ee,A5){0xa:0x1} */
    {
        uint32_t addr = F3_A5 + 0x30EF;
        uint8_t b = rd8(m, addr);
        wr8(m, addr, b & (uint8_t)~0x20u);
    }

    /* ROM 0x137974: move.w #0x3a,-(SP) */
    r->a[7] -= 2;
    wr16(m, r->a[7], 0x003au);

    /* ROM 0x137978: jsr 0x0013f238.l */
    /* ROM 0x13797E: jsr 0x000039d8.l */
    Sound_PlayHooked(m, 0x3au);

    /* ROM 0x137984: lea (0x2,SP),SP */
    r->a[7] += 2;

    /* ROM 0x137988: move.w #0x3c,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x003cu);

    /* ROM 0x13798E: move.l #0x137996,(A0) */
    wr32(m, r->a[0], 0x00137996u);

    /* ROM 0x137994: rts */
    return 0;
}
