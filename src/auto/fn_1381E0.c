/* ROM 0x1381E0: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_1381E0(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x1381E0: subq.w #1, (6,A0) */
    uint16_t val = (uint16_t)(rd16(m, r->a[0] + 6) - 1);
    wr16(m, r->a[0] + 6, val);
    if ((int16_t)val > 0) {
        /* ROM 0x1381D8: move.l #0x1381e0,(A0) ; rts */
        wr32(m, r->a[0], 0x1381E0u);
        return 0;
    }

    /* ROM 0x1381E6: move.l (48,A0),D0 */
    r->d[0] = rd32(m, r->a[0] + 0x48);
    if (r->d[0] == 0) goto cleanup;

    /* ROM 0x1381EE: movea.l D0,A1 */
    r->a[1] = r->d[0];
    /* ROM 0x1381F0: clr.l (24,A1) */
    wr32(m, r->a[1] + 0x24, 0);
    /* ROM 0x1381F4: move.b (35,A0),(35,A1) */
    wr8(m, r->a[1] + 0x35, rd8(m, r->a[0] + 0x35));
    /* ROM 0x1381FA: move.l (36,A0),(36,A1) */
    wr32(m, r->a[1] + 0x36, rd32(m, r->a[0] + 0x36));

cleanup:
    /* ROM 0x138200: move.l A0,-(SP) */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);
    /* ROM 0x138202: jsr 0x000105ec.l */
    Obj_Free(m, r->a[0]);
    /* ROM 0x138208: lea (4,SP),SP */
    r->a[7] += 4;
    /* ROM 0x13820C: rts */
    return 0;
}
