#include "obj_BBE06.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
/* ROM 0xBBE06 */
int Obj_BBE06_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBE06 */
    Obj_Animate(m, obj, rd32(m, obj + 0x44u));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BBDFE */
        wr32(m, obj, 0x000BBE06u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
