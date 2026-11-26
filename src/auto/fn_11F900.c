/* ROM 0x11F900: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"

int Fn_11F900(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F900: subq.w #0x1,(0x6,A0) ; bgt.b 0x11F8F8 */
    uint16_t cnt = (uint16_t)rd16(m, r->a[0] + 6u);
    cnt--;
    wr16(m, r->a[0] + 6u, cnt);
    if (cnt > 0) {
        /* ROM 0x11F8F8: move.l #0x11f900,(A0) ; rts */
        wr32(m, r->a[0], 0x0011F900u);
        return 0;
    }

    /* ROM 0x11F906: tst.b (0x30f4,A5) ; bne.w 0x11F9C8 */
    if (rd8(m, F3_A5 + (int16_t)0x30F4u) != 0) {
        goto branch_11F9C8;
    }

    /* ROM 0x11F90E: move.w #0xa,(0x6,A0) ; beq.b 0x11F924 */
    wr16(m, r->a[0] + 6u, 0x000Au);
    /* beq.b never taken (0xA != 0) */

    /* ROM 0x11F916: move.l #0x11f91e,(A0) ; rts */
    wr32(m, r->a[0], 0x0011F91Eu);
    return 0;

branch_11F9C8:
    /* ROM 0x11F9C8: sf (0x3262,A5) ; bfset (0x30ee,A5){0x8:0x1} ; move.w #0x12c,(0x6,A0) */
    wr8(m, F3_A5 + (int16_t)0x3262u, 0u);
    {
        uint8_t bf = rd8(m, F3_A5 + (int16_t)0x30EEu);
        bf |= (1u << 0u); /* bit 8 mod 8 = 0 */
        wr8(m, F3_A5 + (int16_t)0x30EEu, bf);
    }
    wr16(m, r->a[0] + 6u, 0x012Cu);
    /* beq.b never taken */

    /* ROM 0x11F9DA: move.l #0x11f9e2,(A0) ; rts */
    wr32(m, r->a[0], 0x0011F9E2u);
    return 0;

    /* ROM 0x11F924: addq.b #0x1,(0x3262,A5) ; cmpi.b #0x4,(0x3262,A5) ; blt.b 0x11F90E */
    {
        uint8_t state = rd8(m, F3_A5 + (int16_t)0x3262u);
        state++;
        wr8(m, F3_A5 + (int16_t)0x3262u, state);
        if (state < 4) {
            goto branch_11F90E;
        }
    }

    /* ROM 0x11F930: sf (0x3a,A0) ; clr.w (0x3a,A0) ; bfset (0x30ee,A5){0x7:0x1} ; move.w #0xf0,(0x6,A0) */
    wr8(m, r->a[0] + 0x3Au, 0u);
    wr16(m, r->a[0] + 0x3Au, 0u);
    {
        uint8_t bf = rd8(m, F3_A5 + (int16_t)0x30EEu);
        bf |= (1u << 7u);
        wr8(m, F3_A5 + (int16_t)0x30EEu, bf);
    }
    wr16(m, r->a[0] + 6u, 0x00F0u);

    /* ROM 0x11F944: move.l #0x11f94c,(A0) ; rts */
    wr32(m, r->a[0], 0x0011F94Cu);
    return 0;

branch_11F90E:
    /* ROM 0x11F90E: move.w #0xa,(0x6,A0) ; beq.b 0x11F924 */
    wr16(m, r->a[0] + 6u, 0x000Au);
    /* beq.b never taken */

    /* ROM 0x11F916: move.l #0x11f91e,(A0) ; rts */
    wr32(m, r->a[0], 0x0011F91Eu);
    return 0;
}
