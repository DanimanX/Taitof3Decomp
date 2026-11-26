/* ROM 0x184122: drafted by Strata (scripts/strata_fn.py), bit-exact in test_fn. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_184122(const f3mem_t *m, f3regs_t *r) {
    /* ROM 0x184122: movea.l (0x8,A0),A4 */
    r->a[4] = rd32(m, r->a[0] + 8);

    /* ROM 0x184126: move.w (0x50,A4),D0w */
    r->d[0] = rd16(m, r->a[4] + 0x50);

    /* ROM 0x18412A: move.w (0x54,A4),D1w */
    r->d[1] = rd16(m, r->a[4] + 0x54);

    /* ROM 0x18412E: add.w (0x16,A0),D0w */
    {
        int16_t d0 = (int16_t)r->d[0];
        int16_t off = (int16_t)rd16(m, r->a[0] + 0x16);
        r->d[0] = (uint16_t)(d0 + off);
    }

    /* ROM 0x184132: add.w (0x1a,A0),D1w */
    {
        int16_t d1 = (int16_t)r->d[1];
        int16_t off = (int16_t)rd16(m, r->a[0] + 0x1a);
        r->d[1] = (uint16_t)(d1 + off);
    }

    /* ROM 0x184136: move.w D0w,(0x50,A0) */
    wr16(m, r->a[0] + 0x50, (uint16_t)r->d[0]);

    /* ROM 0x18413A: move.w D1w,(0x54,A0) */
    wr16(m, r->a[0] + 0x54, (uint16_t)r->d[1]);

    /* ROM 0x18413E: move.w D0w,(0x14,A0) */
    wr16(m, r->a[0] + 0x14, (uint16_t)r->d[0]);

    /* ROM 0x184142: move.w D1w,(0x18,A0) */
    wr16(m, r->a[0] + 0x18, (uint16_t)r->d[1]);

    /* ROM 0x184146: subq.w #0x1,(0x64,A0) */
    {
        int16_t cnt = (int16_t)rd16(m, r->a[0] + 0x64);
        cnt--;
        wr16(m, r->a[0] + 0x64, (uint16_t)cnt);

        /* ROM 0x18414A: bgt.b 0x0018411a */
        if (cnt > 0) {
            /* ROM 0x18411A: move.l #0x184122,(A0) */
            wr32(m, r->a[0], 0x00184122u);
            /* ROM 0x184120: rts */
            return 0;
        }
    }

    /* ROM 0x18414C: move.l A0,-(SP) */
    r->a[7] -= 4;
    wr32(m, r->a[7], r->a[0]);

    /* ROM 0x18414E: jsr 0x000105ec.l */
    Obj_Free(m, r->a[0]);

    /* ROM 0x184154: lea (0x4,SP),SP */
    r->a[7] += 4;

    /* ROM 0x184158: rts */
    return 0;
}
