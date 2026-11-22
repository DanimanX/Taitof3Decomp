#include "obj_1134E6.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 -> 0x0039D8 */
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

/* ROM 0x1134E6 */
int Obj_1134E6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x50u) & 0x20u) {
        /* 0x1135D2 */
        play_sound(m, 0x0016);
        Obj_Spawn(m, obj, 0x0013E4EEu, 4u, 1u, 5u);
        a1 = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
        if (rd8(m, a1 + 0x50u) & 0x40u)
            Obj_Spawn(m, obj, 0x00117B44u, 3u, 2u, 0x00004E20u);
        wr32(m, obj + 0x00u, 0x00113644u);
        return 0;
    }

    if (rd8(m, a1 + 0x50u) & 0x10u) {
        wr32(m, a1 + 0x14u, rd32(m, a1 + 0x14u) - 0x4000u);
        wr32(m, a1 + 0x18u, rd32(m, a1 + 0x18u) + 0x8000u);
    }

    /* 0x11350E */
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
    if ((rd16(m, F3_A5 + 0x0878u) & 0x0003u) != 0u) {
        wr32(m, obj + 0x00u, 0x001134E6u);
        return 0;
    }

    uint16_t d1w = rd16(m, a1 + 0x06u);
    a1 = rd32(m, obj + 0x08u);
    if (!(rd8(m, a1 + 0x50u) & 0x10u)) {
        if ((rd16(m, F3_A5 + 0x0878u) & 0x0007u) != 0u) {
            wr32(m, obj + 0x00u, 0x001134E6u);
            return 0;
        }
        uint16_t r = (uint16_t)(Rng_NextMem(m) & 0x03FFu);
        if (d1w > r) {
            wr32(m, obj + 0x00u, 0x001134E6u);
            return 0;
        }
    }

    /* 0x11354A */
    uint32_t d1 = 0x00000600u;
    uint16_t dx = (uint16_t)((Rng_NextMem(m) & 0x3Fu) - 0x20u);
    d1 = (d1 & 0x00FFFFFFu) | ((uint32_t)(uint8_t)dx << 24);
    uint16_t dy = (uint16_t)((Rng_NextMem(m) & 0x1Fu) - 0x10u);
    d1 = (d1 & 0xFF00FFFFu) | ((uint32_t)(uint8_t)dy << 16);

    Obj_Spawn(m, obj, 0x0013EE08u, 3u, 2u, d1);
    d1 = (d1 & 0xFFFF0000u) | 0x0002u;     /* 11358E: move.w #2,D1w keeps upper word */
    Obj_Spawn(m, obj, 0x0013E4EEu, 3u, 2u, d1);

    if ((Rng_NextMem(m) & 0x03u) == 0u)
        play_sound(m, 0x0033);

    wr32(m, obj + 0x00u, 0x001134E6u);
    return 0;
}
