#include "obj_F302E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

/* ROM 0x0F302E */
int Obj_F302E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x22u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (rd8(m, obj + 0x21u) == 0u)
        return 0;

    uint8_t c23 = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, c23);
    if (c23 != 0u)
        return 0;

    wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    wr8(m, obj + 0x23u, 0x14u);

    uint32_t a1 = Obj_Spawn(m, obj, 0x000F30B4u, 5u, 2u, 0u);
    if ((int32_t)a1 < 0) {
        wr8(m, obj + 0x22u, (uint8_t)(rd8(m, obj + 0x22u) - 1u));
        return 0;
    }

    wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
    wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x1Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x1Eu));
    wr8(m, a1 + 0x4Eu, 0u);

    if (rd8(m, obj + 0x21u) != 0u)
        return 0;
    wr8(m, a1 + 0x4Eu, rd8(m, obj + 0x4Eu));
    return 0;
}
