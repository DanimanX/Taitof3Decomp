#include "obj_BBD94.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24: step the animation table at A1 (index/timer at +0x3A/+0x3E). */
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

/* ROM 0x0BBD94: draws itself twice on even frames (second copy shifted down
 * 0x50 px, flipped) for a simple afterimage. */
int Obj_BBD94_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD24(m, obj, rd32(m, obj + 0x44u));

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
