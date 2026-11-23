#include "obj_CD646.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
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

/* cancelled when 0x1D58 bit 0 or A5-0x144B bit 4 is set */
static int cancelled(const f3mem_t *m)
{
    return (rd8(m, F3_A5 + 0x1D58u) & 0x01u) || (rd8(m, F3_A5 - 0x144Bu) & 0x10u);
}

/* ROM 0x0CD6EA / 0x0CD6D2 + 0x0CD6FE: stop the sound and free */
static void stop_free(const f3mem_t *m, uint32_t obj, int16_t cmd)
{
    play_sound(m, cmd, 1);
    Obj_Free(m, obj);
}



/* ROM 0x0CBDAE: append to the player-hit list (A5+0x906) */
static void sub_0CBDAE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x0906u, a1 + 4u);
}

/* ROM 0x0CD646: beam, drawn in 16-px segments to the left edge */
int Obj_CD646_Tick(const f3mem_t *m, uint32_t obj)
{
    if (cancelled(m)) {
        stop_free(m, obj, 0x002A);
        return 0;
    }
    const uint32_t tbl = 0x000CD72Eu;   /* lea (0xd2,PC) at 0xCD65A */
    Obj_AnimFrameStep(m, obj, tbl);
    uint16_t d0 = rd16(m, obj + 0x3Au);   /* final offset (+0x3A), as in ROM */
    if ((int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)d0) < 0) {
        stop_free(m, obj, 0x002A);
        return 0;
    }
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, obj + 0x52u)));
    wr16(m, obj + 0x2Cu, 0x0000u);
    wr16(m, obj + 0x30u, 0x0008u);
    wr16(m, obj + 0x2Eu, 0xFFFFu);
    wr16(m, obj + 0x32u, 0x000Eu);
    for (;;) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x10u));
        wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x10u));
        if ((int16_t)rd16(m, obj + 0x14u) <= -0x10)
            break;
        Obj_DrawSprite(m, obj);
    }
    if (rd16(m, obj + 0x3Au) != 0u)
        sub_0CBDAE(m, obj);
    wr32(m, obj + 0x00u, 0x000CD646u);
    return 0;
}
