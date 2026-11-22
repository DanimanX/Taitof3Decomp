#include "obj_CD412.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

/* ROM 0x0CBD24 */
static uint16_t sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        uint16_t d0 = rd16(m, obj + 0x3Au);
        uint32_t entry = a1 + (uint32_t)(int32_t)(int16_t)d0;
        int16_t dur = (int16_t)rd16(m, entry);
        if (dur <= 0)
            return d0;
        uint16_t d1 = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, d1);
        if ((int16_t)d1 <= dur) {
            wr32(m, obj + 0x10u, rd32(m, entry + 2u));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6u));
    }
}

/* ROM 0x0CD412: animated projectile moving along angle +0x20 */
int Obj_CD412_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t d0 = sub_0CBD24(m, obj, a1);
    if ((int16_t)rd16(m, a1 + (uint32_t)(int32_t)(int16_t)d0) <= 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000CD412u);
    return 0;
}
