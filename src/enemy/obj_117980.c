#include "obj_117980.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x117980: player option/wingman laser beam & exhaust tick */
int Obj_117980_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p1 = rd32(m, obj + 0x08u);
    uint32_t p2 = rd32(m, p1 + 0x08u);
    uint8_t f50 = rd8(m, p2 + 0x50u);
    if ((f50 & 0x10u) != 0u || (f50 & 0x20u) != 0u ||
        ((f50 & 0x08u) == 0u && (f50 & 0x01u) != 0u) ||
        (rd8(m, F3_A5 + 0x1D58u) & 0x01u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    int16_t t = (int16_t)rd16(m, obj + 0x06u);
    if (t <= 0x78) {
        if (t == 0x78)
            Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x002A));
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, p2 + 0x14u) - 8u));
        wr16(m, obj + 0x18u, rd16(m, p2 + 0x18u));
        int16_t d0 = (int16_t)rd16(m, obj + 0x50u);
        if (rd16(m, obj + 0x22u) == 0u) {
            wr8(m, obj + 0x1Cu, 0u);
            d0 = (int16_t)(d0 - (int16_t)rd16(m, obj + 0x3Eu));
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
            wr16(m, obj + 0x2Cu, 0x0010u);
            wr16(m, obj + 0x2Eu, 0x000Fu);
        } else {
            wr8(m, obj + 0x1Cu, 1u);
            d0 = (int16_t)(d0 - (int16_t)rd16(m, obj + 0x3Eu));
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
            wr16(m, obj + 0x2Cu, 0u);
            wr16(m, obj + 0x2Eu, 0xFFFFu);
        }
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u) - 8u));
        wr32(m, obj + 0x10u, 0x0005ED24u);
        for (;;) {
            int16_t x = (int16_t)rd16(m, obj + 0x14u);
            if (x <= -0x10 || x >= 0x140)
                break;
            Obj_DrawSprite(m, obj);
            if (rd16(m, obj + 0x22u) == 0u) {
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u));
                wr16(m, obj + 0x2Eu, (uint16_t)(rd16(m, obj + 0x2Eu) + 0x0010u));
            } else {
                wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u));
                wr16(m, obj + 0x2Cu, (uint16_t)(rd16(m, obj + 0x2Cu) - 0x0010u));
            }
        }
        if (rd16(m, obj + 0x22u) == 0u) {
            wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
            wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
        } else {
            wr32(m, rd32(m, F3_A5 + 0x0902u), obj);
            wr32(m, F3_A5 + 0x0902u, rd32(m, F3_A5 + 0x0902u) + 4u);
        }
        Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    }
    uint16_t saved_x = rd16(m, obj + 0x14u);
    uint16_t saved_y = rd16(m, obj + 0x18u);
    p1 = rd32(m, obj + 0x08u);
    p2 = rd32(m, p1 + 0x08u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, p2 + 0x14u) - 0x0010u));
    wr16(m, obj + 0x18u, rd16(m, p2 + 0x18u));
    if (rd16(m, obj + 0x22u) == 0u) {
        wr8(m, obj + 0x1Cu, 0u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    } else {
        wr8(m, obj + 0x1Cu, 1u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, obj + 0x50u)));
    }
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u) - 0x000Fu));
    wr32(m, obj + 0x10u, rd32(m, 0x00117B34u + (uint32_t)rd16(m, obj + 0x3Au) * 4u));
    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x18u, saved_y);
    wr16(m, obj + 0x14u, saved_x);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + 0x00008000u);
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) & 3u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + 0x00020000u);
    wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) & 7u));
    int16_t next = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)next);
    if (next > 0) {
        wr32(m, obj + 0x00u, 0x00117980u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
