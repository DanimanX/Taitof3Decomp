#include "obj_BB952.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0BB952: drifting sprite with a shrinking zoom byte, own step at
 * obj+0x58 frames. */
int Obj_BB952_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BB952 */
    if (rd8(m, obj + 0x1Fu) != 0u) {
        wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) - 1u));
        if ((int16_t)rd16(m, obj + 0x5Cu) < 0) {
            wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 1u));
            wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 1u));
            wr16(m, obj + 0x5Cu, rd16(m, obj + 0x58u));
        }
    }

    /* 0x0BB970 */
    Obj_Animate(m, obj, rd32(m, obj + 0x0Cu));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    uint16_t d1w = (uint16_t)((uint32_t)rd16(m, obj + 0x54u) >> 1);   /* asr.w #1 */
    uint32_t d = (uint32_t)rd8(m, obj + 0x1Fu) * (uint32_t)d1w;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (d << 8));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    d1w = (uint16_t)((uint32_t)rd16(m, obj + 0x56u) >> 1);
    d = (uint32_t)rd8(m, obj + 0x1Eu) * (uint32_t)d1w;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (d << 8));
    Obj_DrawSprite(m, obj);

    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + rd32(m, obj + 0x4Cu));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + rd32(m, obj + 0x50u));
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BB952u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
