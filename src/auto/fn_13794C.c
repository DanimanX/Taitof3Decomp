/* ROM 0x13794C: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_13794C(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13794C: subq.w #1, (6,A0) */
    uint16_t val = (uint16_t)(rd16(m, r->a[0] + 6u) - 1u);
    wr16(m, r->a[0] + 6u, val);
    /* ROM 0x137950: bgt.b 0x137944 */
    if ((int16_t)val > 0) {
        /* ROM 0x137944: move.l #0x13794c,(A0) */
        wr32(m, r->a[0], 0x0013794Cu);
        /* ROM 0x13794A: rts */
        return 0;
    }
    /* ROM 0x137952: bfset (0x30ee,A5){0xb:0x1} */
    {
        uint32_t addr = F3_A5 + 0x30eeu;
        uint8_t v = rd8(m, addr);
        v |= (uint8_t)(1u << (0xbu & 7u));
        wr8(m, addr, v);
    }
    /* ROM 0x137958: move.w #0x1e,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x001eu);
    /* ROM 0x13795E: beq.b 0x13796e */
    if (val == 0) {
        goto branch_13796e;
    }
    /* ROM 0x137960: move.l #0x137968,(A0) */
    wr32(m, r->a[0], 0x00137968u);
    /* ROM 0x137966: rts */
    return 0;

branch_13796e:
    /* ROM 0x13796E: bfclr (0x30ee,A5){0xa:0x1} */
    {
        uint32_t addr = F3_A5 + 0x30eeu;
        uint8_t v = rd8(m, addr);
        v &= (uint8_t)~(1u << (0xau & 7u));
        wr8(m, addr, v);
    }
    /* ROM 0x137974: move.w #0x3a,-(SP) */
    r->a[7] -= 2u;
    wr16(m, r->a[7], 0x003au);
    /* ROM 0x137978: jsr 0x13f238.l */
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x0013797Eu);
    Sound_PlayHooked(m, 0x003au);
    r->a[7] += 4u;
    /* ROM 0x13797E: jsr 0x39d8.l */
    r->a[7] -= 4u;
    wr32(m, r->a[7], 0x00137984u);
    /* 0x39D8 already sent by Sound_PlayHooked (13F238 + 39D8) */
    r->a[7] += 4u;
    /* ROM 0x137984: lea (0x2,SP),SP */
    r->a[7] += 2u;
    /* ROM 0x137988: move.w #0x3c,(0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x003cu);
    /* ROM 0x13798E: move.l #0x137996,(A0) */
    wr32(m, r->a[0], 0x00137996u);
    /* ROM 0x137994: rts */
    return 0;
}
