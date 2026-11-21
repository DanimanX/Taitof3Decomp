#include "obj_BD0EC.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"
#include "../audio/sound_cmd.h"

/* ROM 0x13F238: sound command remap */
static int16_t sub_13F238(const f3mem_t *m, int16_t cmd)
{
    if (rd8(m, F3_A5 + 0x3306u) != 0u && cmd >= 1 && cmd <= 0x40)
        cmd = (int16_t)(uint16_t)((uint16_t)cmd + 0x53u);
    return cmd;
}

/* ROM 0x0BD0EC: enemy "shrink and die" (scale byte obj+0x1E/0x1F, both down
 * 4 per frame while the decel phase lasts). Plain subq/move.b/Mn imports. */
int Obj_BD0EC_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BD0EC: vertical deceleration (0x100 - zoom)^2 */
    uint16_t d0w = 0x0100u;
    d0w = (uint16_t)(d0w - (uint16_t)rd8(m, obj + 0x1Fu));
    int32_t d0 = (int32_t)(int16_t)d0w * (int32_t)(int16_t)d0w;   /* muls.w */
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) - (uint32_t)d0);

    /* 0x0BD0FE */
    uint16_t t = rd16(m, obj + 0x4Cu);
    uint16_t u = rd16(m, obj + 0x60u);
    if ((int16_t)t < (int16_t)u)
        goto check_bounds;                        /* blt */
    if ((int16_t)t == (int16_t)u) {
        int16_t cmd = sub_13F238(m, 0x2F);
        if (m->dpram) {
            Sound_PlayCmd(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmd(&tmp_m, cmd);
        }
    }

    /* 0x0BD122 */
    if (rd8(m, obj + 0x1Fu) != 0u) {
        uint16_t dx = (uint16_t)(rd16(m, obj + 0x44u) - 0x0068u);
        uint16_t dy = (uint16_t)((uint16_t)(rd16(m, obj + 0x48u) - 0x003Cu) << 3);
        dy = (uint16_t)(dy & 0xFFC0u);
        dx = (uint16_t)((uint32_t)(int32_t)(int16_t)dx >> 4);    /* asr.w #4 */
        uint16_t idx = (uint16_t)(dx + dy);
        uint8_t ang = rd8(m, 0x0030CCu + (uint32_t)(int32_t)(int16_t)idx);
        uint16_t d1w = ang;
        uint16_t a = (uint16_t)((uint16_t)(ang + 0x60u) & 0xFFu);
        uint16_t s0 = rd16(m, 0x0027E40u + (uint32_t)a * 2u);
        uint16_t s1 = rd16(m, 0x0027E40u + (uint32_t)d1w * 2u);
        s0 = (uint16_t)(s0 + 0x20u);
        uint16_t d2w = (uint16_t)(int16_t)(int8_t)(uint8_t)~rd8(m, obj + 0x1Fu);
        int32_t v = (int32_t)(int16_t)d2w * (int32_t)(int16_t)d2w;
        d2w = (uint16_t)((uint32_t)v >> 8);
        int32_t x = (int32_t)(int16_t)s0 * (int32_t)(int16_t)d2w;
        int32_t y = (int32_t)(int16_t)s1 * (int32_t)(int16_t)d2w;
        x >>= 2;
        y >>= 1;
        wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + (uint32_t)x);
        wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + (uint32_t)y);

        /* 0x0BD190: up to 4 shrink steps */
        uint16_t d7w = 3;
        for (;;) {
            wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 1u));
            wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 1u));
            if (rd8(m, obj + 0x1Eu) == 0u)
                break;
            if (d7w == 0)
                break;
            d7w--;
        }
    }

check_bounds:
    /* 0x0BD1A4 */
    if ((int32_t)rd32(m, obj + 0x44u) > 0x01B00000)
        goto die;
    if ((int32_t)rd32(m, obj + 0x48u) > 0x01580000)
        goto die;

    /* 0x0BD1D0 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    uint32_t d = (uint32_t)rd8(m, obj + 0x1Fu) * 0x38u;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (d << 8));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    d = (uint32_t)rd8(m, obj + 0x1Eu) * 0x38u;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (d << 8));
    Obj_Animate(m, obj, rd32(m, obj + 0x5Cu));
    Obj_DrawSprite(m, obj);
    t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BD0ECu);
        return 0;
    }

die:
    /* 0x0BD218 */
    {
        int16_t cmd = sub_13F238(m, 0x2F);
        if (m->dpram) {
            Sound_PlayCmdForced(m, cmd);
        } else {
            uint8_t tmp_dpram[F3_DPRAM_SIZE] = {0};
            f3mem_t tmp_m = *m;
            tmp_m.dpram = tmp_dpram;
            Sound_PlayCmdForced(&tmp_m, cmd);
        }
    }
    Obj_Free(m, obj);
    return 0;
}
