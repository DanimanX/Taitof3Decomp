#include "obj_CDD12.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x0CDD12 -> 0x0CDCF2: one-shot animation from table +0x58 */
int Obj_CDD12_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x58u);
    Obj_AnimFrameStep(m, obj, a1);
    uint16_t d0 = rd16(m, obj + 0x3Au);   /* final offset (+0x3A), as in ROM */
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CDD12u);
    return 0;
}
