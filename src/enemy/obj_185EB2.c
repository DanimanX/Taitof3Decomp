#include "obj_185EB2.h"
#include "../objects/obj_pool.h"

/* ROM 0x185EB2: head of the 0x185F0E volley emitter (the tail lives at
 * 0x185F0E and is decompiled in obj_185F0E.c; its parent re-set at 0x185EFE
 * lands back here). Picks +0x44 from the 0x0CC2BC modulo of A5+0x878 by the
 * 0x180-A5+0x87E range scaled by 10, arms +0x48 with 60, and while the
 * budget holds: follows its parent's +0x50/+0x54 and fires the four packed
 * 0x17D136 shots (group 9, count 2) from the long table at 0x185F22,
 * reloading +0x06 from +0x44. With the budget spent it parks at 0x185F42
 * (one tick, +0x06 = 1); if the first spawn in that last volley failed
 * (Obj_Spawn returned 0), it frees itself instead. */

/* ROM 0x0CC2BC: D0w = (A5+0x878) mod max(1, ((0x180 - A5+0x87E) * d0) >> 8).
 * mulu.w is unsigned; lsl.l #8 then swap keeps the high half of the product,
 * which is D1.w. Same helper as sub_0CC2BC in obj_12EB90.c. */
/* D1.w on return: (0x180 - difficulty) * d0 >> 8, at least 1 */
static uint16_t sub_0CC2BC_d1(const f3mem_t *m, uint16_t d0w)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1w = (uint16_t)(d1 >> 16);
    if (d1w == 0u)
        d1w = 1u;
    return d1w;
}

int Obj_185EB2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x185EB2: move.w #0xA,D0 ; jsr 0x0CC2BC ; move.w D1,(0x44,A0) ;
     * move.w #0x3C,(0x48,A0) */
    /* the caller keeps D1 (the divisor, >= 1), not the D0 remainder */
    wr16(m, obj + 0x44u, sub_0CC2BC_d1(m, 10u));
    wr16(m, obj + 0x48u, 0x003Cu);

    for (;;) {
        /* 0x185EC6: follow the parent */
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
        wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));

        /* 0x185EDA: D7 = 3..0 ; move.l (0x185F22,PC,D7.w*4),D1 ; spawn ;
         * dbf. The long table is at 0x185F22 (extension word address) and
         * holds +0xEC1700C0, +0xF81C00C0, +0x02100C0C, +0x082600C0. */
        for (int d7 = 3; d7 >= 0; d7--) {
            uint32_t d1 = rd32(m, 0x00185F22u + (uint32_t)d7 * 4u);
            (void)Obj_Spawn(m, obj, 0x0017D136u, 9u, 2u, d1);
        }

        /* 0x185EFE: move.w (0x44,A0),(0x6,A0) ; beq 0x185F14 */
        uint16_t reload = rd16(m, obj + 0x44u);
        wr16(m, obj + 0x06u, reload);
        if (reload != 0u) {
            /* 0x185F06 */
            wr32(m, obj + 0x00u, 0x00185F0Eu);
            return 0;
        }

        /* 0x185F14 */
        for (;;) {
            int16_t left = (int16_t)(rd16(m, obj + 0x48u) - rd16(m, obj + 0x44u));
            wr16(m, obj + 0x48u, (uint16_t)left);
            if (left > 0)
                break;                              /* bgt 0x185EC6 */

            /* 0x185F32: move.w #1,(0x6,A0) ; beq 0x185F48 never taken */
            wr16(m, obj + 0x06u, 0x0001u);
            wr32(m, obj + 0x00u, 0x00185F42u);      /* 0x185F3A */
            return 0;
        }
    }
}
