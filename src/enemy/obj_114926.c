#include "obj_114926.h"
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

/* ROM 0x113652: init from the spawn argument (+0xC: dx.b, dy.b, angle.b, flip.b)
 * at the parent's position */
static void sub_113652(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 24));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 16));
    wr16(m, obj + 0x20u, (uint16_t)(((d0 >> 8) & 0xFFu) << 8));
    wr16(m, obj + 0x22u, (uint16_t)(d0 & 0xFFu));
    if ((d0 & 0xFFu) != 0u)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
}

/* ROM 0x114926: init */
int Obj_114926_Tick(const f3mem_t *m, uint32_t obj)
{
    play_sound(m, 0x002A);
    wr16(m, obj + 0x2Cu, 0x0000u);
    wr16(m, obj + 0x2Eu, 0x001Fu);
    wr16(m, obj + 0x30u, 0x0006u);
    wr16(m, obj + 0x32u, 0x0009u);
    wr32(m, obj + 0x10u, 0x0002AD78u);
    sub_113652(m, obj);
    wr8(m, obj + 0x1Du, 0x06u);
    wr32(m, obj + 0x24u, 0x03200000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x42u, 0x0018u);
    wr32(m, obj + 0x00u, 0x00114984u);
    return 0;
}
