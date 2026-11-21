#include "obj_BC4A8.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0BC4A8: sprite moving along obj+0x44 (per-frame x delta) for
 * obj+0x60 frames. */
int Obj_BC4A8_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x44u));
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BC4A8u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
