#include "obj_BBE50.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x0BBE50: blink (16 on / 16 off) with a sound, for +0x60 frames */
int Obj_BBE50_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = rd16(m, obj + 0x60u);
    if ((t & 0x10u) == 0u) {
        if ((t & 0x0Fu) == 0x0Fu) {
            int16_t cmd = sub_13F238(m, 0x002C);
            if (m->dpram) {
                Sound_PlayCmd(m, cmd);
            } else {
                uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
                f3mem_t tmp_m = *m;
                tmp_m.dpram = tmp_dpram;
                Sound_PlayCmd(&tmp_m, cmd);
            }
        }
        Obj_DrawSprite(m, obj);
    }
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BBE50u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
