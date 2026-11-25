#include "obj_F7CDC.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x0F7CDC */
int Obj_F7CDC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x3E8u);
            uint32_t p = rd32(m, obj + 0x08u);
            uint8_t cnt = (uint8_t)(rd8(m, p + 0x20u) - 1u);
            wr8(m, p + 0x20u, cnt);
            if (cnt == 0u && (rd8(m, p + 0x07u) & 0x02u) == 0u) {
                if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                    Obj_StartDying(m, obj);
                    return 0;
                }
                Obj_RelayTimer(m, 0x2710u);
                (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x2710u);
            }
            wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
            Obj_DieWithBonusDbgChk(m, obj, 0u);
            return 0;
        }
    }

    int goto_die = 0;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x3E8u);
        goto_die = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        goto_die = 1;
    }

    if (goto_die) {
        uint32_t expl = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
        if ((int32_t)expl >= 0) {
            wr32(m, expl + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, expl + 0x14u, (uint16_t)(rd16(m, expl + 0x14u) + 0x0018u));
            wr32(m, expl + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, expl + 0x18u, (uint16_t)(rd16(m, expl + 0x18u) + 0x0010u));
            wr16(m, expl + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, expl + 0x60u, rd16(m, obj + 0x60u));
        }
        uint32_t p = rd32(m, obj + 0x08u);
        uint8_t cnt = (uint8_t)(rd8(m, p + 0x20u) - 1u);
        wr8(m, p + 0x20u, cnt);
        if (cnt == 0u && (rd8(m, p + 0x07u) & 0x02u) == 0u) {
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            Obj_0D0E04(m, obj, 0x2710u);
            (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0x2710u);
        }
        uint32_t tail = Obj_Spawn(m, obj, 0x000F8158u, 5u, 2u, 0u);
        if ((int32_t)tail >= 0) {
            wr32(m, tail + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, tail + 0x14u, (uint16_t)(rd16(m, tail + 0x14u) + 0x0020u));
            wr32(m, tail + 0x18u, rd32(m, obj + 0x18u));
            wr8(m, tail + 0x4Eu, rd8(m, obj + 0x4Eu));
            wr32(m, tail + 0x08u, p);
        } else {
            wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
            wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x01u));
        }
        Obj_DieWithBonusChk(m, obj, 0u);
        return 0;
    }

    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    uint8_t f7 = rd8(m, obj + 0x07u);
    wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
    if ((f7 & 0x02u) != 0u) {
        if (ObjScript_CF1E4(m, obj) < 0)
            return -1;
    }
    if (rd8(m, obj + 0x20u) != 0u) {
        if (ObjScript_CF4B8(m, obj, 0x000F8040u) < 0)
            return -1;
    }
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        wr8(m, p + 0x20u, (uint8_t)(rd8(m, p + 0x20u) - 1u));
        wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
        wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x03u));
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u: {
        uint8_t t = rd8(m, obj + 0x23u);
        int check_players = 0;
        if (t != 0u) {
            t--;
            wr8(m, obj + 0x23u, t);
            if (t != 0u)
                check_players = 1;
        }
        if (!check_players) {
            Obj_AimAtPlayers(m, obj, 0x000F8040u);
            if (rd8(m, obj + 0x23u) != 0u) {
                uint32_t shot = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
                if ((int32_t)shot >= 0) {
                    wr32(m, shot + 0x14u, rd32(m, obj + 0x14u));
                    wr16(m, shot + 0x14u, (uint16_t)(rd16(m, shot + 0x14u) + 0x0002u));
                    wr32(m, shot + 0x18u, rd32(m, obj + 0x18u));
                    wr16(m, shot + 0x18u, (uint16_t)(rd16(m, shot + 0x18u) + 0x000Fu));
                    wr16(m, shot + 0x58u, rd16(m, obj + 0x42u));
                    wr16(m, shot + 0x10u, 0u);
                }
            }
        }
        int near_y = 0;
        static const uint32_t pl[2] = { 0x41B190u, 0x41B21Au };
        for (int i = 0; i < 2; i++) {
            if ((int16_t)rd16(m, pl[i] + 0x4Cu) < 0)
                continue;
            int16_t dy = (int16_t)(rd16(m, obj + 0x18u) - rd16(m, pl[i] + 0x18u) + 1);
            if (dy < 0)
                dy = (int16_t)-dy;
            if ((uint16_t)dy <= 0x0023u) {
                near_y = 1;
                break;
            }
        }
        if (near_y) {
            Obj_SetAnim(m, obj, 0x000F8120u);
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
            wr8(m, obj + 0x06u, 0x01u);
        }
        break;
    }
    case 1u:
        break;
    default:
        return -1;
    }

    Obj_AnimStep(m, obj);
    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);

    wr8(m, obj + 0x1Cu, 0x01u);
    uint32_t saved_spr = rd32(m, obj + 0x10u);
    uint16_t d58 = rd16(m, obj + 0x58u);
    if (d58 == 0x0064u) {
        wr32(m, obj + 0x10u, 0x00062F54u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0022u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0006u));
    } else if (d58 == 0x0104u) {
        wr32(m, obj + 0x10u, 0x00062F7Cu);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0022u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0006u));
    } else {
        wr32(m, obj + 0x10u, 0x00062F68u);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0024u));
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x10u, saved_spr);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    wr8(m, obj + 0x1Cu, 0u);
    return 0;
}
