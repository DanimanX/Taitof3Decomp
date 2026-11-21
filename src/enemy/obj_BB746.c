#include "obj_BB746.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0BB746 */
int Obj_BB746_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, obj + 0x44u));
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + rd32(m, obj + 0x48u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4240u));
    Obj_Animate(m, obj, 0x000BB786u);
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, obj + 0x14u) >= -0x40) {
        wr32(m, obj + 0x00u, 0x000BB746u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
