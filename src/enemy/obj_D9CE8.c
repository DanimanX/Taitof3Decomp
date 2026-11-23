#include "obj_D9CE8.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

#define ASR16(v, n) ((int16_t)(((int16_t)(v) < 0) \
    ? ~(uint16_t)((uint16_t)~(uint16_t)(v) >> (n)) \
    : ((uint16_t)(v) >> (n))))

#define ASR32(v, n) ((int32_t)(((int32_t)(v) < 0) \
    ? ~(uint32_t)((uint32_t)~(uint32_t)(v) >> (n)) \
    : ((uint32_t)(v) >> (n))))

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* bomb active and on screen: bomb-scatter tail (jmp 0xCBB2C) */
static int bombed(const f3mem_t *m, uint32_t obj)
{
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    return (rd8(m, F3_A5 + 0x1D58u) & 0x02u) && x >= -0x0A && x <= 0x011C;
}

/* ROM 0x0D9CE8: bullet: move by velocity, die on terrain / off screen;
 * when shot (or on the stage-clear flag) spawn 0xDBDA2 */
int Obj_D9CE8_Tick(const f3mem_t *m, uint32_t obj)
{
    if (bombed(m, obj)) {
        Obj_ExplodeTick(m, obj);
        return 0;
    }
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
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x14u) + 8u),
                                   (int16_t)(rd16(m, obj + 0x18u) + 8u)) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
