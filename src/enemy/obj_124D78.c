#include "obj_124D78.h"
#include "../objects/obj_pool.h"

/* ROM 0x124D78: run the per-slot sub-object handlers from the 4-byte pointer
 * table at 0x124DB4, but only the slots enabled in the bit mask A5+0x30F2
 * (btst.b D0). The mask is then mirrored into +0x22 and the object loops to
 * 0x124D70. When the parent flag (A1+0x59) is set the object is freed.
 * The slot handlers themselves are not decompiled, so an enabled slot makes
 * the tick return -1 before any side effect of that path (A0/A1/D0 are kept
 * across the jsr exactly like the ROM's movem save/restore).
 * (see work/disasm/bundle_124D78.s) */
int Obj_124D78_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x59u) != 0u) {
        /* 0x124DB4: jsr 0x0105EC */
        Obj_Free(m, obj);
        return 0;
    }

    /* 0x124D84 */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 1u));

    uint8_t mask = rd8(m, F3_A5 + 0x30F2u);
    for (uint16_t d0 = 0u; d0 < 8u; d0++) {
        /* 0x124D90: lea (0x36,PC),A1 -> 0x124DB4; movea.l (A1)+,A2 */
        (void)rd32(m, 0x00124DB4u + (uint32_t)d0 * 4u);
        /* 0x124D92: btst.b D0,(0x30f2,A5); beq 0x124DA4 */
        if (mask & (uint8_t)(1u << (d0 & 7u)))
            return -1;                 /* jsr (A2): handler not decompiled */
    }

    /* 0x124DAC: move.b (0x30f2,A5),(0x22,A0) */
    wr8(m, obj + 0x22u, mask);
    /* 0x124D70 */
    wr32(m, obj + 0x00u, 0x00124D78u);
    return 0;
}
