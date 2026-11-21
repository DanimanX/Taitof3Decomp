#include "obj_BC008.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0BC008: animate + draw for +0x60 frames, then free */
int Obj_BC008_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, 0x000BC02Eu);
    Obj_DrawSprite(m, obj);
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BC008u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
