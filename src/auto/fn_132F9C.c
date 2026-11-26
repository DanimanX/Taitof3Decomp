/* ROM 0x132F9C: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_132F9C(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x132F9C: subq.w #0x1,(0x6,A0) */
    uint16_t cnt = rd16(m, r->a[0] + 6);
    cnt--;
    wr16(m, r->a[0] + 6, cnt);
    /* ROM 0x132FA0: bgt.b 0x132F94 */
    if (cnt > 0) {
        wr32(m, r->a[0], 0x132F9Cu);
        return 0;
    }

    /* ROM 0x132FA2: cmpi.w #0x514,(0x30da,A5) */
    uint16_t val = rd16(m, F3_A5 + 0x30DAu);
    /* ROM 0x132FA8: ble.w 0x13301C */
    if (val <= 0x514u) {
        /* ROM 0x13301C: move.w #0x13,-(SP) */
        r->a[7] -= 2;
        wr16(m, r->a[7], 0x13u);
        /* ROM 0x133020: jsr 0x13F238 */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x133026u);
        Sound_PlayHooked(m, 0x13u);
        r->a[7] += 4;
        /* ROM 0x133026: jsr 0x39D8 */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x13302Cu);
        Sound_PlayCmd(m, 0x13);
        r->a[7] += 4;
        /* ROM 0x13302C: lea 2(SP),SP */
        r->a[7] += 2;
        /* ROM 0x133030: bfclr (0x30ee,A5){0x6:0x1} -> bit 9 of word */
        uint16_t w = rd16(m, F3_A5 + 0x30EEu);
        w &= (uint16_t)~(1u << 9);
        wr16(m, F3_A5 + 0x30EEu, w);
        /* ROM 0x133036: move.w #0x3c,(0x6,A0) */
        wr16(m, r->a[0] + 6, 0x3Cu);
        /* ROM 0x13303C: beq.b 0x13304C */
        if (val == 0x514u) {
            /* ROM 0x13304C: move.w #0x3c,(0x6,A0) */
            wr16(m, r->a[0] + 6, 0x3Cu);
            /* ROM 0x133052: move.l #0x13305a,(A0) */
            wr32(m, r->a[0], 0x13305Au);
        } else {
            /* ROM 0x13303E: move.l #0x133046,(A0) */
            wr32(m, r->a[0], 0x133046u);
        }
        return 0;
    }

    /* ROM 0x132FAC: bfclr (0x30ee,A5){0xB:0x1} -> bit 4 of word */
    uint16_t w2 = rd16(m, F3_A5 + 0x30EEu);
    w2 &= (uint16_t)~(1u << 4);
    wr16(m, F3_A5 + 0x30EEu, w2);
    /* ROM 0x132FB2: move.b #0x6,(0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 0x6u);
    /* ROM 0x132FB8: bfset (0x30ee,A5){0xC:0x1} -> bit 3 of word */
    uint16_t w3 = rd16(m, F3_A5 + 0x30EEu);
    w3 |= (uint16_t)(1u << 3);
    wr16(m, F3_A5 + 0x30EEu, w3);
    /* ROM 0x132FBE: move.l #0x132fc6,(A0) */
    wr32(m, r->a[0], 0x132FC6u);
    return 0;
}
