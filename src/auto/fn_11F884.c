/* ROM 0x11F884: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_11F884(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F884: subq.w #0x1,(0x6,A0) */
    uint16_t val = rd16(m, r->a[0] + 6);
    val--;
    wr16(m, r->a[0] + 6, val);

    /* ROM 0x11F888: bgt.b 0x0011f87c */
    if (val > 0) {
        /* ROM 0x11F87C: move.l #0x11f884,(A0) ; rts */
        wr32(m, r->a[0], 0x0011F884u);
        return 0;
    }

    /* ROM 0x11F88A: move.w #0x2f,-(SP) */
    r->a[7] -= 2;
    wr16(m, r->a[7], 0x2fu);

    /* ROM 0x11F88E: jsr 0x0013f238.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x0011F894u);
    Sound_PlayHooked(m, 0x2f);
    r->a[7] += 4;

    /* ROM 0x11F894: jsr 0x00003b66.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x0011F89Au);
    Sound_PlayCmdForced(m, 0);
    r->a[7] += 4;

    /* ROM 0x11F89A: lea (0x2,SP),SP */
    r->a[7] += 2;

    /* ROM 0x11F89E: move.w #0x1e,(0x6,A0) */
    wr16(m, r->a[0] + 6, 0x1eu);

    /* ROM 0x11F8A4: beq.b 0x0011f8b4 (Z from subq.w) */
    if (val == 0) {
        /* ROM 0x11F8B4: bfset (0x30ee,A5){0x1:0x1} */
        uint8_t v = rd8(m, F3_A5 + 0x30EEu);
        v |= (1 << 1);
        wr8(m, F3_A5 + 0x30EEu, v);

        /* ROM 0x11F8BA: move.w #0xa,(0x6,A0) */
        wr16(m, r->a[0] + 6, 0xa);

        /* ROM 0x11F8C0: beq.b 0x0011f8d0 */
        if (val == 0) {
            /* ROM 0x11F8D0: bfclr (0x30ee,A5){0x5:0x1} */
            v = rd8(m, F3_A5 + 0x30EEu);
            v &= ~(1 << 5);
            wr8(m, F3_A5 + 0x30EEu, v);

            /* ROM 0x11F8D6: bra.w 0x0011f7b6 */
            /* ROM 0x11F7B6: move.l #0x11f7be,(A0) ; rts */
            wr32(m, r->a[0], 0x0011F7BEu);
            return 0;
        }

        /* ROM 0x11F8C2: move.l #0x11f8ca,(A0) */
        wr32(m, r->a[0], 0x0011F8CAu);
        return 0;
    }

    /* ROM 0x11F8A6: move.l #0x11f8ae,(A0) */
    wr32(m, r->a[0], 0x0011F8AEu);
    return 0;
}
