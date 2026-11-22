#include "obj_124466.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 pre-hook + Sound_PlayCmd, with a scratch DPRAM when the
 * harness has none */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
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

/* ROM 0x12446C: stop the sound, clear the flags and chain to 0x1244A2 */
static void sub_12446C(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 + 0x30F3u, 0u);            /* sf */
    play_sound(m, 0x0013);                  /* 0x124470 */
    wr8(m, obj + 0x3Au, 0u);                /* sf (0x3a,A0) */
    wr16(m, obj + 0x06u, 0x0003u);
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x80u)); /* bfset {0:1} */
    wr16(m, obj + 0x3Eu, 0x001Eu);
    wr32(m, obj + 0x00u, 0x001244A2u);      /* 0x12449A */
}

/* ROM 0x124466: let the 0x50-tick window run out, then finish */
int Obj_124466_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t c = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, c);
    if ((int16_t)c > 0) {
        wr32(m, obj + 0x00u, 0x00124466u);  /* 0x12445E: stay */
        return 0;
    }
    sub_12446C(m, obj);
    return 0;
}
