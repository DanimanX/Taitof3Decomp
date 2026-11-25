#include "obj_D6958.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D6958: homing missile flight phase */
int Obj_D6958_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_fx;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_fx:
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, Obj_13E4D0(m, obj, 1u));
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0032));
        wr32(m, obj + 0x00u, 0x000D6ACAu);
        return 0;
    }
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) + 0x0800u);
    uint32_t dir2 = (ang >> 14) & 3u;
    uint32_t spr_idx = (ang >> 12) & 7u;
    wr32(m, obj + 0x10u, rd32(m, 0x000D6B04u + spr_idx * 4u));
    wr8(m, obj + 0x1Cu, rd8(m, 0x000D6B24u + dir2));
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
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x20u) >> 8));
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    if ((int16_t)rd16(m, obj + 0x42u) < (int16_t)rd16(m, obj + 0x22u)) {
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
    } else {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) + 1u);
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t < 0x012C) {
            uint16_t want = Obj_0CC3C6(m, obj, rd32(m, obj + 0x3Eu));
            int8_t diff = (int8_t)((uint8_t)want - rd8(m, obj + 0x20u));
            if (diff < -2)
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0280u));
            else if (diff > 2)
                wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0280u));
            else
                wr8(m, obj + 0x20u, (uint8_t)(rd8(m, obj + 0x20u) + (diff >> 1)));
        }
    }
    if ((rd16(m, obj + 0x06u) & 7u) == 0u)
        (void)Obj_Spawn(m, obj, 0x00118B3Cu, 9u, 2u, 0u);
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x <= -0x10 || y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
        wr32(m, obj + 0x00u, 0x000D6ACAu);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x000D6958u);
    return 0;
}
