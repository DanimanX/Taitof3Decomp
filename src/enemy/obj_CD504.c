#include "obj_CD504.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 remap + Sound_PlayCmd */
static void play_sound(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    f3mem_t tmp_m = *m;
    uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
    if (!m->dpram)
        tmp_m.dpram = tmp_dpram;
    Sound_PlayCmd(&tmp_m, cmd);
}

/* ROM 0x0CBD86: clear position/animation state */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x0CD504: init from the spawn argument (+0xC: dx.b, dy.b, flags.w) */
int Obj_CD504_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    wr16(m, obj + 0x28u, 0x0001u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 24));
    wr16(m, obj + 0x52u, (uint16_t)((int16_t)(int8_t)(uint8_t)(d0 >> 16) - 0x0C));
    if ((uint16_t)d0 == 0u) {
        wr8(m, obj + 0x1Du, 0x2Eu);
        play_sound(m, 0x0029);
    }
    wr16(m, obj + 0x06u, 0x0028u);
    wr32(m, obj + 0x00u, 0x000CD556u);
    return 0;
}
