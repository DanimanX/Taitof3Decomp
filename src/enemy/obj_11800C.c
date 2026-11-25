#include "obj_11800C.h"
#include "../objects/obj_pool.h"

/* ROM 0x11800C: damage-over-time cloud. Every tick it adds 0x10000 to the
 * area-damage slot of its target (0x0C660A: 0x41A980[target +0x22]), and
 * frees itself when the +0x06 count runs out. */
int Obj_11800C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0C660A: movea.l D1,A0 ; move.w (0x22,A0),D1 ; add.l D0,(0x41A980,D1.w*4) */
    uint32_t tgt = rd32(m, obj + 0x36u);
    uint32_t slot = 0x0041A980u + (uint32_t)(int32_t)(int16_t)rd16(m, tgt + 0x22u) * 4u;
    wr32(m, slot, rd32(m, slot) + 0x00010000u);

    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0011800Cu);          /* 0x118004 */
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
