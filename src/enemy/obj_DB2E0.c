#include "obj_DB2E0.h"
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

/* shot down or stage cleared: spawn the explosion fn and die */
static int killed(const f3mem_t *m, uint32_t obj, uint32_t fn)
{
    if ((int32_t)rd32(m, obj + 0x24u) > 0 && !(rd8(m, F3_A5 - 0x144Bu) & 0x20u))
        return 0;
    Obj_Spawn(m, obj, fn, 9u, 2u, 0u);
    Obj_StartDying(m, obj);
    return 1;
}

static int hits_terrain(const f3mem_t *m, uint16_t x, uint16_t y)
{
    return (uint8_t)Terrain_TestPoint(m, (int16_t)x, (int16_t)y) != 0u;
}

/* ROM 0x13F238 + 0x39D8: sound command (+0x53 in the alternate sound mode) */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x0DB2E0: missile growing out of the parent (+0x1F zoom), then
 * launched (+6 set) along its velocity */
int Obj_DB2E0_Tick(const f3mem_t *m, uint32_t obj)
{
    if (bombed(m, obj)) {
        Obj_ExplodeTick(m, obj);
        return 0;
    }
    if (killed(m, obj, 0x000DBDA2u))
        return 0;
    if (rd8(m, obj + 0x06u) == 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        if (rd8(m, p + 0x06u) == 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        uint8_t z = (uint8_t)(rd8(m, obj + 0x1Fu) - 0x10u);
        wr8(m, obj + 0x1Fu, z);
        if (z == 0u) {
            wr16(m, obj + 0x2Eu, 0x1Cu);
            wr8(m, obj + 0x06u, 1u);
            Coll_AddHazard(m, obj);
            Obj_DrawSprite(m, obj);
            play_sound(m, 0x2A);
            return 0;
        }
        wr32(m, obj + 0x3Au, rd32(m, p + 0x14u));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x58u)));
        wr32(m, obj + 0x3Eu, rd32(m, p + 0x18u));
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x60u)));
        uint16_t d0 = (uint16_t)(0x20u - (z >> 3));
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - d0));
        wr16(m, obj + 0x2Eu, d0);
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        Coll_AddHazard(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
    Obj_FlagOffscreen(m, obj);
    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (hits_terrain(m, (uint16_t)(rd16(m, obj + 0x3Au) + 0x10u), (uint16_t)(rd16(m, obj + 0x3Eu) + 7u))) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
