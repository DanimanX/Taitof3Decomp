/* ROM 0x11F012: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../kernel/rng.h"

int Fn_11F012(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x11F012: btst.b #1, (0x1d58,A5) */
    uint16_t v = rd16(m, F3_A5 + 0x1D58u);
    if ((v & 2u) == 0u) goto L_11F038;

    /* ROM 0x11F01A: jsr 0x00001d8a.l */
    r->d[0] = Rng_Next();

    /* ROM 0x11F020: andi.w #3, D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | (r->d[0] & 0x3u);

    /* ROM 0x11F024: subq.w #1, D0w */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | ((uint16_t)r->d[0] - 1u);

    /* ROM 0x11F026: cmpi.w #0xa0, (0x30d2,A5) */
    uint16_t a5_30d2 = rd16(m, F3_A5 + 0x30D2u);

    /* ROM 0x11F02C: ble.b 0x0011f034 */
    if ((int16_t)a5_30d2 <= 0xA0) goto L_11F034;

    /* ROM 0x11F02E: sub.w D0w, (0x30d2,A5) */
    wr16(m, F3_A5 + 0x30D2u, a5_30d2 - (uint16_t)r->d[0]);
    goto L_11F038;

L_11F034:
    /* ROM 0x11F034: add.w D0w, (0x30d2,A5) */
    wr16(m, F3_A5 + 0x30D2u, a5_30d2 + (uint16_t)r->d[0]);

L_11F038:
    /* ROM 0x11F038: subq.w #1, (0x6,A0) */
    uint16_t a0_6 = rd16(m, r->a[0] + 6u);
    a0_6--;
    wr16(m, r->a[0] + 6u, a0_6);

    /* ROM 0x11F03C: bgt.b 0x0011f00a */
    if (a0_6 > 0) {
        /* ROM 0x11F00A: move.l #0x11f012,(A0) */
        wr32(m, r->a[0], 0x0011F012u);
        /* ROM 0x11F010: rts */
        return 0;
    }

    /* ROM 0x11F03E: move.b #1, (0x30f2,A5) */
    wr8(m, F3_A5 + 0x30F2u, 1u);

    /* ROM 0x11F044: move.w #0x1a4, (0x6,A0) */
    wr16(m, r->a[0] + 6u, 0x1A4u);

    /* ROM 0x11F04A: move.l #0x11f052,(A0) */
    wr32(m, r->a[0], 0x0011F052u);

    /* ROM 0x11F050: rts */
    return 0;
}
