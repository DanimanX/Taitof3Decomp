#include "obj_CDDA6.h"
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

/* ROM 0x0CCC08: apply the spawn argument (+0x0C) offsets and angle,
 * place at the parent's position */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 8) & 0xFFu));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
}

/* ROM 0x0CDDA6 (-> 0x0CDDB6): explosion init */
int Obj_CDDA6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 4u);
    play_sound(m, 0x0011u);
    wr16(m, obj + 0x50u, 0xFFF0u);
    wr16(m, obj + 0x52u, 0xFFF0u);
    sub_0CCC08(m, obj);
    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x1Du, 0x2Au);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x00u, 0x000CDDF8u);
    return 0;
}
