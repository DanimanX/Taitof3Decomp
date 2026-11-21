#include "obj_BCC96.h"
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

/* ROM 0x0BCC96 */
int Obj_BCC96_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x1Fu) == 0xC0u) {
        uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) + 1u);
        wr16(m, obj + 0x60u, t);
        if ((int16_t)t > 0x40) {
            /* 0x0BCCFA */
            wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x44u) + 0x28u));
            wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x48u) + 0x20u));
            Obj_Spawn(m, obj, 0x000BCD90u, 0u, 1u, rd32(m, obj + 0x60u));
            play_sound(m, 0x0006u);
            wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00380000u);
            Obj_Spawn(m, obj, 0x000BCE38u, 0u, 1u, rd32(m, obj + 0x48u));
            play_sound(m, 0x0022u);
            wr32(m, obj + 0x00u, 0x000BCD82u);
            return 0;
        }
    } else {
        wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 4u));
        wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 4u));
    }
    /* 0x0BCCBA */
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + rd32(m, obj + 0x4Cu));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (((uint32_t)rd8(m, obj + 0x1Fu) * 0x50u) << 8));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (((uint32_t)rd8(m, obj + 0x1Eu) * 0x40u) << 8));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000BCC96u);
    return 0;
}
