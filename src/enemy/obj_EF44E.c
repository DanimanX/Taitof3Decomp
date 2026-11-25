#include "obj_EF44E.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0EF44E */
int Obj_EF44E_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            wr32(m, obj + 0x10u, 0x0005BBACu);
            Obj_RelayTimer(m, 0x12Cu);
            uint32_t p = rd32(m, obj + 0x08u);
            uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
            wr8(m, p + 0x22u, cnt);
            if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
                Obj_DieWithBonusDbgChk(m, obj, 0u);
                return 0;
            }
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_RelayTimer(m, 0xBB8u);
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0xBB8u);
            Obj_DieWithBonusDbg(m, obj, 0xBB8u);
            return 0;
        }
    }

    int goto_die = 0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x12Cu);
        goto_die = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        goto_die = 1;
    }

    if (goto_die) {
        uint32_t expl = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
        if ((int32_t)expl >= 0) {
            wr32(m, expl + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, expl + 0x14u, (uint16_t)(rd16(m, expl + 0x14u) + 0x0014u));
            wr32(m, expl + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, expl + 0x18u, (uint16_t)(rd16(m, expl + 0x18u) + 0x0010u));
            wr16(m, expl + 0x58u, 0u);
            wr16(m, expl + 0x60u, 0u);
        }
        uint32_t p = rd32(m, obj + 0x08u);
        uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
        wr8(m, p + 0x22u, cnt);
        if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
            Obj_DieWithBonusChk(m, obj, 0u);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        Obj_0D0E04(m, obj, 0xBB8u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0xBB8u);
        Obj_DieWithBonus(m, obj, 0xBB8u);
        return 0;
    }

    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u));
    Obj_AnimStep(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
        wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    switch (rd16(m, obj + 0x68u)) {
    case 0u:
        break;
    case 1u:
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
        break;
    case 2u:
        wr16(m, obj + 0x2Cu, 0x000Cu);
        wr16(m, obj + 0x2Eu, 0x0019u);
        wr16(m, obj + 0x30u, 0x0007u);
        wr16(m, obj + 0x32u, 0x0018u);
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x03u));
        wr8(m, obj + 0x22u, 0x01u);
        break;
    case 3u: {
        uint32_t shot = Obj_Spawn(m, obj, 0x000DA20Eu, 9u, 2u, 0u);
        if ((int32_t)shot >= 0) {
            wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 0x0018u));
            wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x0010u));
        }
        break;
    }
    case 4u:
        wr8(m, obj + 0x22u, 0u);
        break;
    case 5u:
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u));
        break;
    default:
        return -1;
    }

    if (rd8(m, obj + 0x22u) != 0u)
        Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
