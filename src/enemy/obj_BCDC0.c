#include "obj_BCDC0.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0BCDC0: animation plus a linear x drift (obj+0x44, +1.0 per frame). */
int Obj_BCDC0_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_AnimFrameStep(m, obj, rd32(m, obj + 0x5Cu));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, obj + 0x44u));
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + 0x00010000u);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        wr32(m, obj + 0x00u, 0x000BCDC0u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
