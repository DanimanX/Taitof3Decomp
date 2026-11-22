#include "obj_119062.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 + 0x39D8 */
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

/* ROM 0x119062: every 0x50 frames play sound 0x36, +0x22 times, then free */
int Obj_119062_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    if ((int16_t)n > 0) {
        wr32(m, obj + 0x00u, 0x00119062u);
        return 0;
    }
    /* 0x119068: subq.w; bhi -> old value >= 2 */
    uint16_t c = rd16(m, obj + 0x22u);
    wr16(m, obj + 0x22u, (uint16_t)(c - 1u));
    if (c >= 2u) {
        play_sound(m, 0x36);
        wr16(m, obj + 0x06u, 0x50u);
        wr32(m, obj + 0x00u, 0x00119062u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
