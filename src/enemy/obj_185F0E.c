#include "obj_185F0E.h"
#include "../objects/obj_pool.h"

/* ROM 0x185F0E: volley emitter. Counts +0x06 down; at zero it takes +0x44 off
 * the +0x48 budget and, while budget remains, follows the parent and fires a
 * ring of four 0x17D136 shots (group 9, count 2) with the packed offsets at
 * 0x185F22, reloading +0x06 from +0x44. With the budget spent it hands over
 * to the one-tick 0x185F42 wait. */
int Obj_185F0E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x185F0E: subq.w #1,(0x6,A0) ; bgt 0x185F06 */
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x00185F0Eu);          /* 0x185F06 */
        return 0;
    }

    for (;;) {
        /* 0x185F14: move.w (0x44,A0),D0 ; sub.w D0,(0x48,A0) ; bgt 0x185EC6 */
        int16_t left = (int16_t)(rd16(m, obj + 0x48u) - rd16(m, obj + 0x44u));
        wr16(m, obj + 0x48u, (uint16_t)left);
        if (left <= 0) {
            /* 0x185F32: move.w #1,(0x6,A0) ; beq never taken -> 0x185F3A */
            wr16(m, obj + 0x06u, 0x0001u);
            wr32(m, obj + 0x00u, 0x00185F42u);
            return 0;
        }

        /* 0x185EC6: follow the parent */
        uint32_t a4 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
        wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));
        /* 0x185EDA: D7 = 3..0 ; move.l (0x185F22,PC,D7.w*4),D1 ; spawn ; dbf */
        for (int d7 = 3; d7 >= 0; d7--) {
            uint32_t d1 = rd32(m, 0x00185F22u + (uint32_t)d7 * 4u);
            (void)Obj_Spawn(m, obj, 0x0017D136u, 9u, 2u, d1);
        }
        /* 0x185EFE: move.w (0x44,A0),(0x6,A0) ; beq 0x185F14 */
        uint16_t reload = rd16(m, obj + 0x44u);
        wr16(m, obj + 0x06u, reload);
        if (reload != 0u) {
            wr32(m, obj + 0x00u, 0x00185F0Eu);      /* 0x185F06 */
            return 0;
        }
    }
}
