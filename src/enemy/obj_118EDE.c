#include "obj_118EDE.h"
#include "../objects/obj_pool.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0CBD24: advance the {dur.w, frame.l} animation at a1 by +0x3A/+0x3E */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    uint16_t d0;

    for (;;) {
        d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x118EDE: animate (script 0x118F10) while moving; free at its end */
int Obj_118EDE_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x00118F10u;
    uint16_t d0 = sub_0CBD24(m, obj, a1);
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_MoveByAngle(m, obj, 0u);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x00118EDEu);
    return 0;
}
