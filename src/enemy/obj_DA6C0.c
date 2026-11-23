#include "obj_DA6C0.h"
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

/* ROM 0x13F238 -> 0x0039D8 */
static void play_sound(const f3mem_t *m, uint16_t cmdw)
{
    int16_t cmd = (int16_t)cmdw;
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    if (m->dpram) {
        Sound_PlayCmd(m, cmd);
    } else {
        uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
        f3mem_t tmp_m = *m;
        tmp_m.dpram = tmp_dpram;
        Sound_PlayCmd(&tmp_m, cmd);
    }
}

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0DA6C0 */
int Obj_DA6C0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            Obj_ExplodeTick(m, obj);             /* jmp 0x0CBB2C */
            return 0;
        }
    }

    /* 0x0DA6DE */
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        /* 0x0DA7E2 */
        Obj_Spawn(m, obj, 0x000DBDA2u, 9u, 2u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }

    if (rd8(m, obj + 0x06u) != 0u) {
        /* 0x0DA786: flying free */
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + rd32(m, obj + 0x50u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));
        Obj_FlagOffscreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        if (Terrain_TestPoint(m, (int16_t)(uint16_t)(rd16(m, obj + 0x3Au) + 0x10u),
                              (int16_t)(uint16_t)(rd16(m, obj + 0x3Eu) + 0x07u))) {
            Obj_StartDying(m, obj);
            return 0;
        }
        wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
        wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
        Coll_AddHazard(m, obj);
        Obj_DrawSprite(m, obj);
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    uint8_t t = (uint8_t)(rd8(m, obj + 0x1Fu) - 8u);
    wr8(m, obj + 0x1Fu, t);
    if (t == 0u) {
        /* 0x0DA758: launch */
        wr16(m, obj + 0x2Eu, 0x001Cu);
        wr8(m, obj + 0x06u, 0x01u);
        Coll_AddHazard(m, obj);
        Obj_DrawSprite(m, obj);
        play_sound(m, 0x002Au);
        return 0;
    }

    /* 0x0DA70E: held by the parent */
    wr32(m, obj + 0x3Au, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + rd16(m, obj + 0x58u)));
    wr32(m, obj + 0x3Eu, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) + rd16(m, obj + 0x60u)));
    uint16_t d0 = (uint16_t)(0x20u - (uint16_t)(t >> 3));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) - d0));
    wr16(m, obj + 0x2Eu, d0);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
