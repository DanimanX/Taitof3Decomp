#include "obj_BBEC0.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x0BBEC0 */
int Obj_BBEC0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBEC0 */
    if ((int16_t)rd16(m, obj + 0x60u) < 0x28 &&
        (rd16(m, obj + 0x60u) & 0x0008u) == 0u) {
        if ((rd16(m, obj + 0x60u) & 0x0007u) == 0x0007u) {
            /* 0x0BBEE6: SFX 0x1D */
            int16_t cmd = 0x001D;
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
        Obj_DrawSprite(m, obj);             /* 0x0BBEFA */
    }

    /* 0x0BBF00 */
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, d0);
    if ((int16_t)d0 >= 0) {
        wr32(m, obj, 0x000BBEC0u);          /* 0x0BBEB8 */
        return 0;
    }

    Obj_Free(m, obj);                       /* 0x0BBF06 */
    return 0;
}
