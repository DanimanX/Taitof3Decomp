#include "obj_CD9F6.h"
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

/* ROM 0x0CBDDE: A1 = owner to credit */
static uint32_t sub_0CBDDE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) != 2u && rd8(m, obj + 0x35u) == 1u)
        a1 = rd32(m, a1 + 0x08u);
    return a1;
}

/* ROM 0x006422 */

/* ROM 0x00649A */

/* ROM 0x0CBB2C */

/* ROM 0x0D7306 */

/* ROM 0x0CD3D6: switch to the follow-up record 0x0CD444[D0] (optional
 * sound word first), restart animation, next state 0x0CD412 */
static void next_record(const f3mem_t *m, uint32_t obj, uint16_t d0)
{
    uint32_t a1 = rd32(m, 0x000CD444u + (uint32_t)(int32_t)(int16_t)d0 * 4u);
    uint16_t snd = rd16(m, a1); a1 += 2u;
    if (snd != 0u)
        play_sound(m, snd);
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x00u, 0x000CD412u);
}

/* ROM 0x0CD9F6: homing shot; switches to the turn state 0x0CDAC8 when
 * level with the target (+0x0C) */
int Obj_CD9F6_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = sub_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        next_record(m, obj, rd16(m, obj + 0x54u));
        return 0;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        next_record(m, obj, rd16(m, obj + 0x54u));
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    if ((uint8_t)Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x14u) + 8u),
                                   (int16_t)(rd16(m, obj + 0x18u) + 8u)) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    {   /* ROM 0x0CBDAE */
        uint32_t a1 = rd32(m, F3_A5 + 0x906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x906u, a1 + 4u);
    }
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    Obj_DrawSprite(m, obj);
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u), y = (int16_t)rd16(m, obj + 0x18u);
        if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u) ||
            y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
            Obj_Free(m, obj);
            return 0;
        }
    }
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    int16_t dx = (int16_t)(uint16_t)(rd16(m, a1 + 0x14u) - rd16(m, obj + 0x14u));
    if (dx > -8 || dx < -0x18) {
        wr32(m, obj + 0x00u, 0x000CD9F6u);
        return 0;
    }
    wr16(m, obj + 0x22u, rd16(m, obj + 0x20u));
    if ((int16_t)(uint16_t)(rd16(m, a1 + 0x18u) + 8u) > (int16_t)rd16(m, obj + 0x18u))
        wr16(m, obj + 0x20u, 0x0040u);
    else
        wr16(m, obj + 0x20u, 0x00C0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0x0100u);
    wr32(m, obj + 0x00u, 0x000CDAC8u);
    return 0;
}
