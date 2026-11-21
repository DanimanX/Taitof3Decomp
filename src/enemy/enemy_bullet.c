#include "enemy_bullet.h"
#include "../player/player_move.h"
#include "../video/sprite.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"

/* ROM 0x102FE6 */
int Obj_EnemyBullet_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            /* unimplemented: ROM 0x0CBB2C */
            return -1;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||
        (rd8(m, F3_A5 - 0x144Bu) & 0x20u)) {
        Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
    uint32_t dy = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + dy);

    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    int16_t tx = (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x0Cu));
    int16_t ty = (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x0Eu));
    if (Terrain_TestPoint(m, tx, ty)) {
        Obj_StartDying(m, obj);
        return 0;
    }

    wr8(m, obj + 0x1Du, 0u);
    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x21u) - 1u);
    wr8(m, obj + 0x21u, cnt);
    if (cnt == 0u) {
        wr8(m, obj + 0x21u, 0x04u);
        wr8(m, obj + 0x1Du, 0x42u);
    }

    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
