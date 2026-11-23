#include "obj_BBD94.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0BBD94: draws itself twice on even frames (second copy shifted down
 * 0x50 px, flipped) for a simple afterimage. */
int Obj_BBD94_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_AnimFrameStep(m, obj, rd32(m, obj + 0x44u));

    if ((rd16(m, obj + 0x60u) & 0x01u) == 0u) {
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x50u));
        wr8(m, obj + 0x1Cu, 0x02u);
        Obj_DrawSprite(m, obj);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x50u));
        wr8(m, obj + 0x1Cu, 0u);
    }

    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BBD94u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
