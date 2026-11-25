#include "obj_11B668.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x11B668: homing/curved projectile tick */
int Obj_11B668_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_0cd3d6;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
    die_0cd3d6:
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
        uint32_t a1 = rd32(m, 0x000CD444u + 2u * 4u);
        uint16_t snd = rd16(m, a1);
        a1 += 2u;
        if (snd != 0u)
            Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)snd));
        wr32(m, obj + 0x0Cu, a1);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x1Cu, 0u);
        wr32(m, obj + 0x00u, 0x000CD412u);
        return 0;
    }
    uint16_t want = Obj_0CC3C6(m, obj, rd32(m, obj + 0x0Cu));
    uint16_t turn = rd16(m, obj + 0x3Eu);
    if (turn != 0u) {
        int8_t diff = (int8_t)((uint8_t)want - rd8(m, obj + 0x20u));
        if (diff < 0)
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - turn));
        else if (diff > 0)
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + turn));
        wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x3Eu) - 0x0Cu));
    }
    wr8(m, obj + 0x43u, rd8(m, obj + 0x3Au));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + (rd16(m, obj + 0x3Au) >> 6)));
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    uint16_t d0 = (uint16_t)((uint8_t)(rd16(m, obj + 0x20u) >> 8) + 8u);
    uint32_t dir2 = (d0 >> 6) & 3u;
    uint32_t spr_idx = (d0 >> 4) & 7u;
    wr32(m, obj + 0x10u, rd32(m, 0x0011B76Cu + spr_idx * 4u));
    wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + dir2));
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            wr32(m, obj + 0x10u, 0x0005D84Cu);
            Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_DrawSprite(m, obj);
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u) ||
        y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x0011B668u);
    return 0;
}
