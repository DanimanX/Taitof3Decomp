#include "obj_C7E5E.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

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

/* ROM 0x0C7E5E: random effect spawns on the live players */
int Obj_C7E5E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t r = (uint16_t)(Rng_NextMem(m) & 0xFFu);
    if ((int16_t)r <= (int16_t)rd16(m, obj + 0x22u)) {
        uint32_t a1 = 0x0041B190u;
        for (int i = 0; i < 2; i++, a1 += 0x8Au) {
            if ((int16_t)rd16(m, a1 + 0x4Cu) < 0 || (rd8(m, a1 + 0x65u) & 0x04u) != 0u)
                continue;
            Obj_Spawn(m, obj, 0x000C80DCu, 3u, 4u, a1);
        }
    }
    if (rd16(m, obj + 0x06u) == 0x009Eu)
        play_sound(m, 0x0027u);
    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 1u));
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000C7E5Eu);
        return 0;
    }
    wr16(m, obj + 0x06u, 0x001Eu);
    wr32(m, obj + 0x00u, 0x000C7EE6u);
    return 0;
}
