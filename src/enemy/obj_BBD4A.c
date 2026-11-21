#include "obj_BBD4A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0BBD4A: animate + draw for +0x60 frames, then free */
int Obj_BBD4A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, rd32(m, obj + 0x44u));
    Obj_DrawSprite(m, obj);
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BBD4Au);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
