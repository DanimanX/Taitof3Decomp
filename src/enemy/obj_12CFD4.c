#include "obj_12CFD4.h"
#include "../objects/obj_pool.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238 remap + Sound_PlayCmd / Sound_PlayCmdForced */
static void play_sound(const f3mem_t *m, int16_t cmd, int forced)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(cmd + 0x53);
    f3mem_t tmp_m = *m;
    uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
    if (!m->dpram)
        tmp_m.dpram = tmp_dpram;
    if (forced)
        Sound_PlayCmdForced(&tmp_m, cmd);
    else
        Sound_PlayCmd(&tmp_m, cmd);
}

/* ROM 0x12CFD4: wait for the boss timer / hit points, then start its death */
int Obj_12CFD4_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 + 0x329Du, 0u);
    if ((int16_t)rd16(m, F3_A5 + 0x3318u) <= 0x3840 && (int32_t)rd32(m, F3_A5 + 0x30DAu) > 0) {
        wr32(m, obj + 0x00u, 0x0012CFD4u);
        return 0;
    }

    /* 0x12CFE8 */
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x10u));
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);
    wr8(m, F3_A5 + 0x30F2u, (uint8_t)(rd8(m, F3_A5 + 0x30F2u) | 0x01u));
    play_sound(m, 0x001F, 1);
    Obj_Free(m, rd32(m, F3_A5 + 0x30DEu));

    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
    uint32_t d0 = 5u;
    uint16_t d1w = (uint16_t)(0u - (uint16_t)((uint16_t)rd8(m, a1 + 0x1Eu) - 0x0100u));
    uint16_t d2w = d1w;
    a1 = rd32(m, a1 + 0x10u);
    int32_t d1 = (int32_t)(int16_t)d1w * (int16_t)rd16(m, a1);
    d1 >>= 5;
    d0 = (d0 & 0xFF00FFFFu) | (((uint32_t)d1 & 0xFFu) << 16);   /* bfins D1,D0{8:8} */
    int32_t d2 = (int32_t)(int16_t)d2w * (int16_t)rd16(m, a1 + 2u);
    d2 >>= 5;
    d0 = (d0 & 0x00FFFFFFu) | (((uint32_t)d2 & 0xFFu) << 24);   /* bfins D2,D0{0:8} */
    Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0);

    play_sound(m, 0x0016, 0);
    if (rd16(m, F3_A5 + 0x32DEu) != 0x0007u)
        play_sound(m, 0x0014, 0);

    /* move.w #0xa,(0x6,A0); beq: never taken, so 0x12D0B4.. is dead code */
    wr16(m, obj + 0x06u, 0x000Au);
    wr32(m, obj + 0x00u, 0x0012D0AEu);
    return 0;
}
