#include "obj_BCDC0.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24: step the animation table at A1 by the index/timer in
 * obj+0x3A/+0x3E. D0.w on return is the loop counter (last evaluated index),
 * not a table lookup. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        uint16_t d0w = rd16(m, obj + 0x3Au);
        if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0w) <= 0)
            return;
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + 1u));
        uint16_t d1w = rd16(m, obj + 0x3Eu);
        if ((int16_t)d1w <= (int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0w)) {
            wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)(int32_t)(int16_t)d0w + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 6u));
    }
}

/* ROM 0x0BCDC0: animation plus a linear x drift (obj+0x44, +1.0 per frame). */
int Obj_BCDC0_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD24(m, obj, rd32(m, obj + 0x5Cu));
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
