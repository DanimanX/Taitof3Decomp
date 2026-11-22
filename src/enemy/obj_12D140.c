#include "obj_12D140.h"
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

/* ROM 0x12D140 */
int Obj_12D140_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    if ((int16_t)rd16(m, a1 + 0x56u) > 0x28) {
        wr32(m, obj + 0x00u, 0x0012D140u);   /* 0x12D138 */
        return 0;
    }
    play_sound(m, 0x002Bu);
    wr16(m, F3_A5 + 0x32A6u, 0x0014u);
    wr16(m, F3_A5 + 0x32A4u, 0x1100u);
    wr16(m, F3_A5 + 0x32A8u, 0x0001u);
    wr32(m, obj + 0x00u, 0x0012D17Eu);       /* 0x12D176 */
    return 0;
}
