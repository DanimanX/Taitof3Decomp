#include "obj_114108.h"
#include "obj_112C3C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
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

/* ROM 0x114108 */
int Obj_114108_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t f = rd8(m, a1 + 0x50u);
    if (f & 0x20u) {
        Obj_Free(m, obj);                   /* 0x114216 */
        return 0;
    }
    if (f & 0x04u) {
        wr32(m, obj + 0x00u, 0x00114108u);  /* 0x114100 */
        return 0;
    }

    Obj_Animate(m, obj, 0x00114224u);       /* lea (0x104,PC) at 0x11411E */
    Obj_113400_FollowParent(m, obj);

    uint16_t d0w = rd16(m, obj + 0x06u);
    if (d0w != 0u) {
        wr16(m, obj + 0x06u, (uint16_t)(d0w - 1u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(d0w >> 2)));
    }

    a1 = rd32(m, obj + 0x08u);
    if (!(rd8(m, a1 + 0x50u) & 0x10u)) {
        uint8_t d0b = (uint8_t)((uint8_t)rd32(m, obj + 0x0Cu) + 1u);
        if (rd8(m, a1 + 0x51u) == d0b) {
            /* 0x114166 */
            play_sound(m, 0x003C);
            wr16(m, obj + 0x06u, 0x001Eu);
            wr16(m, obj + 0x20u, 0x0050u);
            wr32(m, obj + 0x00u, 0x0011418Eu);
            return 0;
        }
    }

    /* 0x11415E */
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x00114108u);
    return 0;
}
