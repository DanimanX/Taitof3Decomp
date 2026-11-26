/* ROM 0x1929E6: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_1929E6(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1929E6: subq.w #1, (6,A0) */
    uint16_t tmp = rd16(m, r->a[0] + 6);
    tmp--;
    wr16(m, r->a[0] + 6, tmp);
    if ((int16_t)tmp > 0) {
        /* ROM 0x1929DE: move.l #0x1929E6, (A0) ; rts */
        wr32(m, r->a[0], 0x1929E6);
        return 0;
    }

    /* ROM 0x1929EC: moveq #0xF, D7 */
    r->d[7] = 0x0000000Fu;

    /* ROM 0x1929EE: loop */
    for (;;) {
        /* ROM 0x1929EE: move.l D7, -(SP) */
        r->a[7] -= 4;
        wr32(m, r->a[7], r->d[7]);
        /* ROM 0x1929F0: move.w #1, -(SP) */
        r->a[7] -= 2;
        wr16(m, r->a[7], 0x1);
        /* ROM 0x1929F4: move.w #0, -(SP) */
        r->a[7] -= 2;
        wr16(m, r->a[7], 0x0);
        /* ROM 0x1929F8: move.l #0x192ABC, -(SP) */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x192ABC);
        /* ROM 0x1929FE: jsr 0x01057C (STOP) */
        return -1;
    }

    /* ROM 0x192A04: lea 12(SP), SP */
    r->a[7] += 0xC;
    /* ROM 0x192A08: dbf D7w, 0x1929EE */
    r->d[7] = (r->d[7] & 0xFFFF0000u) | ((uint16_t)r->d[7] - 1);
    if ((int16_t)r->d[7] != -1) {
        /* loop back */
    }

    /* ROM 0x192A0C: move.l A0, -(SP) */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    /* ROM 0x192A0E: jsr 0x000105EC.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x192A10);
    Obj_Free(m, r->a[0]);
    r->a[7] += 4;

    /* ROM 0x192A14: lea 4(SP), SP */
    r->a[7] += 4;

    /* ROM 0x192A18: rts */
    return 0;
}
