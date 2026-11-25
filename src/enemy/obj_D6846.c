#include "obj_D6846.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D6846: spinning missile/pod launch phase */
int Obj_D6846_Tick(const f3mem_t *m, uint32_t obj)
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
    uint32_t d0 = rd32(m, obj + 0x3Eu);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) & 0x0Fu));
    wr32(m, obj + 0x3Eu, d0 - (d0 >> 5));
    uint16_t w = rd16(m, obj + 0x3Au);
    wr32(m, obj + 0x10u, rd32(m, 0x000D6AE4u + (uint32_t)(w & 7u) * 4u));
    wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + ((w >> 2) & 3u)));
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
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x54u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + rd32(m, obj + 0x58u));
    wr32(m, obj + 0x58u, rd32(m, obj + 0x58u) + 0x00001000u);
    Obj_SubScrollVel(m, obj);
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x <= -0x10 || y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
        wr32(m, obj + 0x00u, 0x000D6ACAu);
        return 0;
    }
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000D6846u);
        return 0;
    }
    wr16(m, obj + 0x42u, 0u);
    int16_t d1 = (int16_t)((rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00u);
    wr16(m, obj + 0x22u, (uint16_t)((((int32_t)0x12 * (int32_t)d1) << 4) >> 16));
    uint16_t ang = (uint16_t)(((rd16(m, obj + 0x3Au) << 12) & 0x7FFFu) - 0x4000u);
    wr16(m, obj + 0x20u, ang);
    wr16(m, obj + 0x2Cu, 1u);
    wr16(m, obj + 0x30u, 1u);
    wr16(m, obj + 0x2Eu, 0x0Eu);
    wr16(m, obj + 0x32u, 0x0Eu);
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x3Eu, rd32(m, F3_A5 + 0x1D28u));
    wr32(m, obj + 0x00u, 0x000D6958u);
    return 0;
}
