/* ROM 0x13314C: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../enemy/obj_E8B72.h"

int Fn_13314C(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x13314C: subq.w #1, (A0+6) */
    uint16_t val = (uint16_t)(rd16(m, r->a[0] + 6) - 1);
    wr16(m, r->a[0] + 6, val);
    if ((int16_t)val > 0) {
        /* ROM 0x133144: move.l #0x13314c,(A0) ; rts */
        wr32(m, r->a[0], 0x0013314C);
        return 0;
    }

    /* ROM 0x133152: cmpi.w #0x320,(A5+0x30DA) */
    if ((int16_t)rd16(m, F3_A5 + 0x30DA) <= 0x320) {
        goto L_133198;
    }

    /* ROM 0x13315C: move.b #4,(A5+0x30F2) */
    wr8(m, F3_A5 + 0x30F2, 4);
    /* ROM 0x133162: bfset (A5+0x30EE){0xC:1} */
    wr8(m, F3_A5 + 0x30EE, rd8(m, F3_A5 + 0x30EE) | (1u << (0xC & 7)));
    /* ROM 0x133168: move.w #0x168,(A0+6) */
    wr16(m, r->a[0] + 6, 0x168);
    /* ROM 0x13316E: beq.b 0x13317E (always taken after move) */
    goto L_13317E;

L_13317E:
    /* ROM 0x13317E: move.b #0,(A5+0x30F2) */
    wr8(m, F3_A5 + 0x30F2, 0);
    /* ROM 0x133184: bfset (A5+0x30EE){0xC:1} */
    wr8(m, F3_A5 + 0x30EE, rd8(m, F3_A5 + 0x30EE) | (1u << (0xC & 7)));
    /* ROM 0x13318A: cmpi.w #0x320,(A5+0x30DA) */
    if ((int16_t)rd16(m, F3_A5 + 0x30DA) <= 0x320) {
        goto L_133198;
    }
    /* ROM 0x133194: bra.w 0x13304C */
    goto L_13304C;

L_13304C:
    /* ROM 0x13304C: move.w #0x3C,(A0+6) */
    wr16(m, r->a[0] + 6, 0x3C);
    /* ROM 0x133052: move.l #0x13305a,(A0) ; rts */
    wr32(m, r->a[0], 0x0013305A);
    return 0;

L_133198:
    /* ROM 0x133198: move.w #0x13,-(SP) */
    r->a[7] -= 2;
    wr16(m, r->a[7], 0x13);
    /* ROM 0x13319C/1331A2: jsr 0x13F238 ; jsr 0x39D8 */
    Sound_PlayHooked(m, 0x13);
    /* ROM 0x1331A8: lea (2,SP),SP */
    r->a[7] += 2;
    /* ROM 0x1331AC: bfset (A5+0x30EE){0xB:1} */
    wr8(m, F3_A5 + 0x30EE, rd8(m, F3_A5 + 0x30EE) | (1u << (0xB & 7)));
    /* ROM 0x1331B2: move.b #7,(A5+0x30F2) */
    wr8(m, F3_A5 + 0x30F2, 7);
    /* ROM 0x1331B8: bfset (A5+0x30EE){0xC:1} */
    wr8(m, F3_A5 + 0x30EE, rd8(m, F3_A5 + 0x30EE) | (1u << (0xC & 7)));
    /* ROM 0x1331BE: move.l #0x1331c6,(A0) */
    wr32(m, r->a[0], 0x001331C6);
    /* ROM 0x1331C4: rts */
    return 0;
}
