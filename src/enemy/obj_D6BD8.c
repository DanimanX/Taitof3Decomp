#include "obj_D6BD8.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D704E..0x0D709E */
static void die_D6BD8(const f3mem_t *m, uint32_t obj, int add_score)
{
    if (add_score) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
    }
    Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u)));
    uint32_t arg = Obj_13E4D0(m, obj, 2u);
    Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, arg);
    wr32(m, obj + 0x00u, 0x000D70A0u);
}

/* ROM 0x0D6BD8 */
int Obj_D6BD8_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        die_D6BD8(m, obj, 1);
        return 0;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        die_D6BD8(m, obj, 0);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    if (x < -120 || x > 500) {
        Obj_Free(m, obj);
        return 0;
    }
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (y < -80 || y > 232) {
        Obj_Free(m, obj);
        return 0;
    }

    Obj_0D0DC2(m, obj);
    Obj_SubScrollVel(m, obj);

    if ((int16_t)rd16(m, obj + 0x58u) < 0) {
        if ((uint8_t)Obj_0D70AE(m, obj, 0x0018, 0x0018) != 0u)
            wr32(m, obj + 0x58u, 0u);
    }

    uint8_t fl = rd8(m, obj + 0x64u);
    int32_t vx = (int32_t)rd32(m, obj + 0x54u);
    if ((fl & 0x01u) != 0u)
        vx = -vx;
    wr32(m, obj + 0x14u, (uint32_t)((int32_t)rd32(m, obj + 0x14u) + vx));

    int32_t vy = (int32_t)rd32(m, obj + 0x58u);
    if ((fl & 0x02u) != 0u)
        vy = -vy;
    wr32(m, obj + 0x18u, (uint32_t)((int32_t)rd32(m, obj + 0x18u) + vy));

    int32_t g = (int32_t)rd32(m, obj + 0x58u) + 0x8000;
    if (g > 0x80000)
        g = 0x80000;
    wr32(m, obj + 0x58u, (uint32_t)g);

    wr32(m, obj + 0x10u, ((int16_t)rd16(m, obj + 0x58u) < 0) ? 0x000651B0u : 0x00065204u);
    wr8(m, obj + 0x1Cu, fl);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    if ((uint8_t)Obj_0D70AE(m, obj, 0x0020, 0x0040) == 0u) {
        wr32(m, obj + 0x00u, 0x000D6BD8u);
        return 0;
    }

    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x000D6CECu);
    return 0;
}
