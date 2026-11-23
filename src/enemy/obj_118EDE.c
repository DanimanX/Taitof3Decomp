#include "obj_118EDE.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x118EDE: animate (script 0x118F10) while moving; free at its end */
int Obj_118EDE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00118F10u;
    uint16_t d0 = rd16(m, obj + 0x3Au);   /* D0w before the step, as in ROM */
    Obj_AnimFrameStep(m, obj, a1);
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_MoveByAngle(m, obj, 0u);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x00118EDEu);
    return 0;
}
