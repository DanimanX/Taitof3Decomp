#include "obj_D9FC8.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0D0690 */
static void sub_0D0690(const f3mem_t *m, uint32_t obj)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0)
            return;
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        a1 += 2u;
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x0D9FC8 */
int Obj_D9FC8_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    /* 0x0D9FE6 */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u)) {
        Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(uint16_t)(rd16(m, obj + 0x14u) + 8u),
                                   (int16_t)(uint16_t)(rd16(m, obj + 0x18u) + 8u)) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    sub_0D0690(m, obj);
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
