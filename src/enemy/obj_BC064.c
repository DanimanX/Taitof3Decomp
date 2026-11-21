#include "obj_BC064.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0BC064: draw for +0x60 frames, then free */
int Obj_BC064_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_DrawSprite(m, obj);
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BC064u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
