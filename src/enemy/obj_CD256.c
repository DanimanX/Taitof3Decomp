#include "obj_CD256.h"
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

/* ROM 0x0CD348 table: sprite selection by mode +0x56 (A1 = +0x58) */
static int sprite_mode(const f3mem_t *m, uint32_t obj, uint16_t mode, uint32_t a1)
{
    uint16_t d0, d1;
    switch (mode) {
    case 0u: Obj_Animate(m, obj, a1); return 0;
    case 1u: wr32(m, obj + 0x10u, rd32(m, a1)); return 0;
    case 2u: case 4u:
        d0 = rd16(m, obj + 0x20u);
        d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + (mode == 2u ? 8u : 4u)));
        d0 = (uint16_t)(d0 >> (mode == 2u ? 4 : 3));
        wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)(int32_t)(int16_t)d0 * 4u));
        return 0;
    case 3u: case 5u:
        d0 = rd16(m, obj + 0x20u);
        d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + (mode == 3u ? 8u : 4u)));
        d1 = (uint16_t)((d0 >> 6) & 3u);
        d0 = (uint16_t)((d0 >> (mode == 3u ? 4 : 3)) & (mode == 3u ? 7u : 0xFu));
        wr32(m, obj + 0x10u, rd32(m, a1 + (uint32_t)d0 * 4u));
        wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + d1));
        return 0;
    }
    return -1;
}

/* ROM 0x0CD256: enemy bullet */
int Obj_CD256_Tick(const f3mem_t *m, uint32_t obj)
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
    uint8_t fl = rd8(m, obj + 0x5Cu);
    if ((fl & 0x01u) != 0u &&
        (uint8_t)Terrain_TestPoint(m, (int16_t)(rd16(m, obj + 0x14u) + 4u),
                                   (int16_t)(rd16(m, obj + 0x18u) + 4u)) != 0u) {
        next_record(m, obj, rd16(m, obj + 0x54u));
        return 0;
    }
    if (rd16(m, obj + 0x56u) > 5u)
        return -1;   /* mode outside the 0x0CD348 table */
    if ((fl & 0x02u) != 0u) {
        uint32_t a1 = rd32(m, F3_A5 + 0x906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x906u, a1 + 4u);
    }
    if ((fl & 0x04u) != 0u) {
        uint32_t a1 = rd32(m, F3_A5 + 0x8FEu);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x8FEu, a1 + 4u);
    }
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));
    sprite_mode(m, obj, rd16(m, obj + 0x56u), rd32(m, obj + 0x58u));
    {   /* ROM 0x11C32C: hit flash */
        if (rd8(m, obj + 0x35u) != 0u) {
            if (rd8(m, obj + 0x1Du) == 6u) {
                wr8(m, obj + 0x35u, 0u);
                wr8(m, obj + 0x1Du, 8u);
            } else {
                wr8(m, obj + 0x1Du, 6u);
            }
        } else {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
        }
    }
    Obj_DrawSprite(m, obj);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);   /* 0x0CBD12 */
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u), y = (int16_t)rd16(m, obj + 0x18u);
        if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u) ||
            y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
            Obj_Free(m, obj);
            return 0;
        }
    }
    wr32(m, obj + 0x00u, 0x000CD256u);
    return 0;
}
