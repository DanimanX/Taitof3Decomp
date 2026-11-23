#include "obj_D64BA.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "obj_E8B72.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D67AC -> 0x0CBD12: relay the script timer and hand over to the
 * explosion tick (0x0CBB2C) with the sprite def at 0x000646C4. Both jump
 * targets live in the not-yet-decompiled 0x0CBBxx/0x0CBCxx graph. */
static int sub_0D67A2(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x000646C4u);
    (void)m;
    (void)obj;
    return -1;
}

/* ROM 0x0D6742: stop the pickup animation and die */
static int sub_0D6742(const f3mem_t *m, uint32_t obj)
{
    /* 0x0D6752 */
    Sound_PlayHooked(m, (uint16_t)rd8(m, obj + 0x65u));
    wr32(m, obj + 0x00u, 0x000D6794u);
    return 0;
}

/* ROM 0x0D64BA */
int Obj_D64BA_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        return sub_0D6742(m, obj);
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        Sound_PlayHooked(m, (uint16_t)rd8(m, obj + 0x65u));
        Obj_Spawn(m, obj, 0x0013E4EEu, 1u, 0u, 0x00000002u);
        wr32(m, obj + 0x00u, 0x000D6794u);
        return 0;
    }

    /* 0x0D64CC */
    wr32(m, obj + 0x10u, 0x000645C8u);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            return sub_0D67A2(m, obj);
        }
    }

    /* 0x0D64F8 */
    Obj_SubScrollVel(m, obj);
    Coll_AddEnemyHazard(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    /* 0x0D6510: cmp.w (0x000CC114).l,D0w = 0x0140 (right screen edge + 0x40) */
    if ((int16_t)rd16(m, obj + 0x14u) >= (int16_t)rd16(m, 0x000CC114u)) {
        wr32(m, obj + 0x00u, 0x000D64BAu);
        return 0;
    }

    /* 0x0D651C: teleport offscreen once +0x5E has built up */
    wr16(m, obj + 0x5Eu, (uint16_t)(rd16(m, obj + 0x5Eu) + rd16(m, obj + 0x60u)));
    if ((int8_t)rd8(m, obj + 0x5Eu) < 0x1E) {
        wr32(m, obj + 0x00u, 0x000D64BAu);
        return 0;
    }

    wr16(m, obj + 0x3Au, 0x0000u);
    wr16(m, obj + 0x3Eu, 0x0000u);
    wr32(m, obj + 0x00u, 0x000D653Cu);
    return 0;
}
