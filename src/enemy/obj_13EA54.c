#include "obj_13EA54.h"
#include "../audio/sound_cmd.h"
#include "../kernel/rng.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/palette.h"

/* ROM 0x13EA54: init projectile/particle from parent, randomize speed and angle,
 * queue palette upload (0x001660), play sound 0x39 on even index, advance to 0x13EB4C. */
int Obj_13EA54_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);

    uint32_t parent = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x22u, rd16(m, parent + 0x22u));
    wr16(m, parent + 0x22u, (uint16_t)(rd16(m, parent + 0x22u) + 1u));

    uint16_t r0 = (uint16_t)((Rng_NextMem(m) & 0xFFu) + 0x40u);
    if ((int16_t)r0 > 0x0100)
        r0 = 0x0100u;
    int32_t spd = ((int32_t)(int16_t)r0 * (int32_t)(int16_t)rd16(m, parent + 0x42u)) >> 8;
    wr16(m, obj + 0x42u, (uint16_t)spd);

    uint32_t arg = rd32(m, obj + 0x0Cu);
    uint32_t d0 = (arg & 0xFFFFFF00u) | ((arg << 4) & 0xFFu);
    wr16(m, obj + 0x20u, (uint16_t)d0);

    uint16_t d1 = (uint16_t)((d0 >> 6) & 3u);
    wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + d1));

    uint32_t tbl_1d = 0x0013ED00u;
    int16_t w32de = (int16_t)rd16(m, F3_A5 + 0x32DEu);
    if (w32de > 3) {
        tbl_1d += 4u;
        if (w32de > 6)
            tbl_1d += 4u;
    }
    wr8(m, obj + 0x1Du, rd8(m, tbl_1d + d1));

    uint16_t angle = (uint8_t)(((Rng_NextMem(m) & 0xFu) - 8u) + rd8(m, obj + 0x21u));
    wr32(m, obj + 0x14u, 0u);
    wr32(m, obj + 0x18u, 0u);
    Obj_MoveByAngle(m, obj, angle);
    wr16(m, obj + 0x2Cu, rd16(m, obj + 0x14u));
    wr16(m, obj + 0x30u, rd16(m, obj + 0x18u));

    uint16_t pal_dst = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Du) << 6) + 0x4004u);
    uint16_t pal_idx = (uint16_t)((rd8(m, obj + 0x21u) >> 4) & 7u);
    pal_dst = (uint16_t)(pal_dst + rd16(m, 0x0013ED0Cu + pal_idx * 2u));
    Pal_QueueUpload(m, 0x00018F84u, pal_dst, 6u);

    if ((rd16(m, obj + 0x22u) & 1u) == 0u) {
        int16_t cmd = Sound_PreCmdHook(m, 0x0039);
        if (m->dpram) {
            Sound_PlayCmd(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmd(&tmp_m, cmd);
        }
    }

    wr32(m, obj + 0x00u, 0x0013EB4Cu);
    return 0;
}
