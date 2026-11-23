#include "obj_BBD00.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBBD00 */
int Obj_BBD00_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBD00: animate from table (A0)+0x44, draw, then age out */
    Obj_AnimFrameStep(m, obj, rd32(m, obj + 0x44u));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BBCF8 */
        wr32(m, obj, 0x000BBD00u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
