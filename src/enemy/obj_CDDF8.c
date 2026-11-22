#include "obj_CDDF8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CDDF8: animated emitter spawning 0xCDE50 */
int Obj_CDDF8_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, 0xCE118u);
    Obj_DrawSprite(m, obj);
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t >= 0) {
        uint32_t d0 = Obj_Spawn(m, obj, 0xCDE50u, 5u, 2u, rd32(m, obj + 0x44u));
        wr32(m, obj + 0x44u, d0);
        if ((int32_t)d0 <= 0)
            wr16(m, obj + 0x06u, 0u);
    }
    if ((int16_t)rd16(m, obj + 0x06u) > -8) {
        wr32(m, obj, 0xCDDF8u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
