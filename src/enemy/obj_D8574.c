#include "obj_D8574.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../player/player_move.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 + 0x39D8: sound command (+0x53 in the alternate sound mode) */
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

static void addw(const f3mem_t *m, uint32_t a, uint16_t v)
{
    wr16(m, a, (uint16_t)(rd16(m, a) + v));
}

/* ROM 0x0D8574: charge glow at the parent for +6 frames; from frame 0x1E a
 * horizontal laser (sprites to the screen edge, hazard box in +0x2C/+0x2E) */
int Obj_D8574_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, obj + 0x08u);
    if (rd32(m, p + 0x28u) == 0u || (rd8(m, F3_A5 + 0x1D58u) & 0x01u)) {
        Obj_Free(m, obj);
        return 0;
    }
    int16_t t = (int16_t)rd16(m, obj + 0x06u);
    if (t <= 0x1E) {
        if (t == 0x1E)
            play_sound(m, 0x2A);
        wr16(m, obj + 0x14u, rd16(m, p + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, p + 0x18u));
        int right = rd8(m, obj + 0x64u) != 0u;
        if (!right) {
            wr8(m, obj + 0x1Cu, 0u);
            addw(m, obj + 0x14u, (uint16_t)(-0x33 - (int16_t)rd16(m, obj + 0x3Eu)));
            wr16(m, obj + 0x2Cu, 0x10u);
            wr16(m, obj + 0x2Eu, 0x0Fu);
        } else {
            wr8(m, obj + 0x1Cu, 1u);
            addw(m, obj + 0x14u, (uint16_t)(0x23u + rd16(m, obj + 0x3Eu)));
            wr16(m, obj + 0x2Cu, 0u);
            wr16(m, obj + 0x2Eu, (uint16_t)-1);
        }
        addw(m, obj + 0x18u, (uint16_t)-0x0A);
        wr32(m, obj + 0x10u, 0x0005ED24u);
        for (;;) {
            int16_t x = (int16_t)rd16(m, obj + 0x14u);
            if (x <= -0x10 || x >= 0x140)
                break;
            Obj_DrawSprite(m, obj);
            if (rd8(m, obj + 0x64u) == 0u) {
                addw(m, obj + 0x14u, (uint16_t)-0x10);
                addw(m, obj + 0x2Eu, 0x10u);
            } else {
                addw(m, obj + 0x14u, 0x10u);
                addw(m, obj + 0x2Cu, (uint16_t)-0x10);
            }
        }
        uint32_t a1 = rd32(m, F3_A5 + 0x906u);
        wr32(m, a1, obj);
        wr32(m, F3_A5 + 0x906u, a1 + 4u);
    }
    /* 0x0D8658: muzzle flash */
    uint16_t sx = rd16(m, obj + 0x14u), sy = rd16(m, obj + 0x18u);
    p = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, p + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, p + 0x18u));
    if (rd8(m, obj + 0x64u) == 0u) {
        wr8(m, obj + 0x1Cu, 0u);
        addw(m, obj + 0x14u, (uint16_t)-0x40);
    } else {
        wr8(m, obj + 0x1Cu, 1u);
        addw(m, obj + 0x14u, 0x20u);
    }
    addw(m, obj + 0x18u, (uint16_t)-0x11);
    uint32_t i = (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x10u, rd32(m, 0x000D86E8u + i * 4u));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x18u, sy);
    wr16(m, obj + 0x14u, sx);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x8000u);
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) & 3u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x20000u);
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) & 7u));
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x000D8574u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
