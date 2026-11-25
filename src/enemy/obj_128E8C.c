#include "obj_128E8C.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x128E8C: homing missile / pod tick */
int Obj_128E8C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
    die_0cd3d6:
        wr8(m, obj + 0x21u, rd8(m, obj + 0x20u));
        wr8(m, obj + 0x20u, 0u);
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
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_0cd3d6;
    }
    uint32_t target = rd32(m, obj + 0x0Cu);
    int16_t d0 = (int16_t)((Obj_0CC3C6(m, obj, target) << 8) - (int16_t)rd16(m, obj + 0x20u));
    int16_t d1 = (int16_t)rd16(m, obj + 0x22u);
    if (d0 <= d1) {
        int16_t neg_d1 = (int16_t)(-d1);
        if (d0 < neg_d1)
            d0 = neg_d1;
    } else {
        d0 = d1;
    }
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + d0));
    wr16(m, obj + 0x22u, (uint16_t)(rd16(m, obj + 0x22u) + 0x22u));
    if ((int16_t)rd16(m, obj + 0x06u) <= 0)
        wr16(m, obj + 0x3Cu, rd16(m, obj + 0x20u));

    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) + 0x0800u);
    uint8_t dir2 = rd8(m, 0x000CC10Eu + ((ang >> 14) & 3u));
    wr8(m, obj + 0x1Cu, (uint8_t)(dir2 ^ 3u));
    uint32_t idx = (((ang >> 12) & 7u) << 1) | ((rd8(m, obj + 0x3Bu) >> 1) & 1u);
    wr32(m, obj + 0x10u, rd32(m, 0x0012902Eu + idx * 4u));
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + 1u));

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
    int16_t cnt = (int16_t)rd16(m, obj + 0x06u);
    if (cnt >= 0) {
        wr16(m, obj + 0x42u, (uint16_t)((uint16_t)cnt >> 2));
    } else {
        wr16(m, obj + 0x42u, (uint16_t)((uint16_t)(-cnt) >> 1));
        wr16(m, obj + 0x22u, 0x0080u);
    }
    Obj_MoveByAngle(m, obj, (uint16_t)(rd16(m, obj + 0x3Cu) >> 8));
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_11C32C(m, obj);
    Obj_DrawSprite(m, obj);

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u) ||
        y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) {
        Obj_Free(m, obj);
        return 0;
    }
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    wr32(m, obj + 0x00u, 0x00128E8Cu);
    return 0;
}
