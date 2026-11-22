#include "obj_11BEF6.h"
#include "../objects/obj_pool.h"

/* ROM 0x11BEF6: tick the parent's (+0x8(A0)) +0x22 countdown, then free the
 * object. The ROM issues two `subq.w #1` in a row (0x11BEFA, guarded by
 * `bgt` to 0x11BF06; 0x11BF02, whose own `bgt` falls through because the
 * `subq` clears Z): both are modelled, so the counter lands exactly where the
 * ROM lands it. */
int Obj_11BEF6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x8u);

    /* 0x11BEFA: subq.w #1,(0x22,A1); bgt.w 0x11BF06 */
    uint16_t c = (uint16_t)(rd16(m, a1 + 0x22u) - 1u);
    wr16(m, a1 + 0x22u, c);
    if ((int16_t)c <= 0) {
        /* 0x11BF02: subq.w #1,(0x22,A1) */
        c = (uint16_t)(rd16(m, a1 + 0x22u) - 1u);
        wr16(m, a1 + 0x22u, c);
    }

    /* 0x11BF06: move.l A0,-(SP); jsr 0x0105EC; lea (4,SP),SP; rts */
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
