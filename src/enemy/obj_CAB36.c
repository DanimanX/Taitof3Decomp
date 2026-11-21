#include "obj_CAB36.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0CBD24 */
static int16_t sub_CBD24(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    for (;;) {
        int16_t d0 = (int16_t)rd16(m, obj + 0x3Au);
        uint32_t ea = (uint32_t)(a1 + (int32_t)d0) & 0xFFFFFFu;
        int16_t v = (int16_t)rd16(m, ea);
        if (v <= 0)
            return d0;
        uint16_t timer = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, timer);
        if ((int16_t)timer <= v) {
            wr32(m, obj + 0x10u, rd32(m, (uint32_t)(ea + 2u) & 0xFFFFFFu));
            return d0;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(d0 + 6));
    }
}

/* ROM 0x0CAB36 */
int Obj_CAB36_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    int16_t d0 = sub_CBD24(m, obj, a1);
    if ((int16_t)rd16(m, (uint32_t)(a1 + (int32_t)d0) & 0xFFFFFFu) <= 0)
        goto lab_0cab9e;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    if (rd8(m, obj + 0x4Cu) == 0u)
        Obj_SubScrollVel(m, obj);

    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));

    uint16_t d0w = rd16(m, obj + 0x42u);
    d0w = (uint16_t)(d0w >> 2);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) - d0w));

    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));

    a1 = rd32(m, obj + 0x10u);
    uint16_t w = (uint16_t)(rd16(m, a1 & 0xFFFFFFu) << 3);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - w));
    w = (uint16_t)(rd16(m, (uint32_t)(a1 + 2u) & 0xFFFFFFu) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - w));

    Obj_DrawSprite(m, obj);

    wr32(m, obj, 0x000CAB36u);
    return 0;

lab_0cab9e:
    Obj_Free(m, obj);
    return 0;
}
