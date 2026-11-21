#include "obj_CA28C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CA28C */
int Obj_CA28C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) > 0) {
        Obj_Animate(m, obj, 0x000CA2CCu);
        uint32_t a1 = rd32(m, F3_A5 + 0x0902u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x0902u, a1 + 4u);
        Obj_DrawSprite(m, obj);
        uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, t);
        if ((int16_t)t > 0) {
            wr32(m, obj + 0x00u, 0x000CA28Cu);
            return 0;
        }
    }
    /* 0x0CA2B6 */
    uint32_t parent = rd32(m, obj + 0x08u);
    wr16(m, parent + 0x5Cu, (uint16_t)(rd16(m, parent + 0x5Cu) + 1u));
    Obj_Free(m, obj);
    return 0;
}
