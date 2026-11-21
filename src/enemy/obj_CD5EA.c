#include "obj_CD5EA.h"
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

/* follow the parent at +0x50/+0x52 (dx - 0x20) and draw */
static void follow_draw(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + rd16(m, obj + 0x50u) - 0x20u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + rd16(m, obj + 0x52u)));
    Obj_DrawSprite(m, obj);
}

/* subq.w #1,(0x6,A0); bhi */
static int count_down(const f3mem_t *m, uint32_t obj)
{
    uint16_t v = rd16(m, obj + 0x06u);
    wr16(m, obj + 0x06u, (uint16_t)(v - 1u));
    return v >= 2u;
}

/* ROM 0x0CD5EA: beam start */
int Obj_CD5EA_Tick(const f3mem_t *m, uint32_t obj)
{
    if (cancelled(m)) {
        stop_free(m, obj, 0x002A);
        return 0;
    }
    Obj_Animate(m, obj, 0x000CD720u);   /* lea (0x120,PC) at 0xCD5FE */
    follow_draw(m, obj);
    if (count_down(m, obj)) {
        wr32(m, obj + 0x00u, 0x000CD5EAu);
        return 0;
    }
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x000CD646u);
    return 0;
}
