/* ROM 0x112ADA: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../video/sprite.h"
#include "../objects/obj_pool.h"

/* ROM 0x112ADA: Check sprite timer, draw sprite, free if needed */
int Fn_112ADA(const f3mem_t *m, f3regs_t *r) {
    /* 112ADA: move.l (0x44,A0),D0 */
    r->d[0] = rd32(m, r->a[0] + 0x44);
    /* 112ADE: sub.l (-0x5d90,A5),D0 */
    r->d[0] -= rd32(m, F3_A5 - 0x5d90);
    /* 112AE2: cmpi.l #0x1400000,D0 */
    /* 112AE8: bge.b 0x00112ad2 */
    if ((int32_t)r->d[0] >= 0x1400000) {
        /* 112AD2: move.l #0x112ada,(A0) */
        wr32(m, r->a[0], 0x112ADA);
        return 0;
    }
    /* 112AEA: move.l D0,(0x14,A0) */
    wr32(m, r->a[0] + 0x14, r->d[0]);
    /* 112AEE: jsr 0x00001282.l */
    r->a[7] -= 4;
    wr32(m, r->a[7], 0x112AF4);
    Obj_DrawSprite(m, r->d[0]);
    r->a[7] += 4;

    /* 112AF4: tst.b (0x3bba,A5) */
    /* 112AF8: bne.w 0x00112afe */
    if (rd8(m, F3_A5 + 0x3bba) != 0) {
        /* 112AFE: move.l A0,-(SP) */
        r->a[7] -= 4;
        wr32(m, r->a[7], r->a[0]);
        /* 112B00: jsr 0x000105ec.l */
        r->a[7] -= 4;
        wr32(m, r->a[7], 0x112B06);
        Obj_Free(m, r->d[0]);
        r->a[7] += 4;
        /* 112B06: lea (0x4,SP),SP */
        r->a[7] += 4;
        return 0;
    } else {
        /* 112AFC: bra.b 0x00112ad2 */
        wr32(m, r->a[0], 0x112ADA);
        return 0;
    }
}
