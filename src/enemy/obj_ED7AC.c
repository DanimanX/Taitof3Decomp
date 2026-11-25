#include "obj_ED7AC.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"
#include "../video/sprite.h"

/* ROM 0x0ED7AC: enemy turret/pod tick */
int Obj_ED7AC_Tick(const f3mem_t *m, uint32_t obj)
{
    const uint32_t k_rec = 0x000EDA1Eu;
    int goto_die = 0;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x12Cu);
            uint32_t p = rd32(m, obj + 0x08u);
            uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
            wr8(m, p + 0x22u, cnt);
            if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
                if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
                    uint8_t d0 = rd8(m, obj + 0x4Eu);
                    if (d0 != 0u)
                        (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, (uint32_t)d0);
                }
            } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) == 0u) {
                Obj_RelayTimer(m, 0xBB8u);
                (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0xBB8u);
                uint8_t d0 = rd8(m, obj + 0x4Eu);
                if (d0 != 0u)
                    (void)Obj_Spawn(m, obj, 0x00117C28u, 0u, 1u, (uint32_t)d0);
            }
            return -1;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        Obj_0D0E04(m, obj, 0x12Cu);
        goto_die = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        goto_die = 1;
    }

    if (goto_die) {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC586u, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x08u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x08u));
            wr16(m, a1 + 0x58u, rd16(m, obj + 0x58u));
            wr16(m, a1 + 0x60u, rd16(m, obj + 0x60u));
        }
        uint32_t p = rd32(m, obj + 0x08u);
        uint8_t cnt = (uint8_t)(rd8(m, p + 0x22u) - 1u);
        wr8(m, p + 0x22u, cnt);
        if (cnt != 0u || (rd8(m, p + 0x07u) & 0x01u) != 0u) {
            Obj_0CEE48(m, obj);
            return 0;
        }
        if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
            Obj_StartDying(m, obj);
            return 0;
        }
        Obj_0D0E04(m, obj, 0xBB8u);
        (void)Obj_Spawn(m, obj, 0x00117B44u, 0u, 1u, 0xBB8u);
        Obj_0CEE48(m, obj);
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
        if (ObjScript_CF4B8(m, obj, k_rec) < 0)
            return -1;
    }
    Obj_ArmOnScreen(m, obj);
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        uint32_t p = rd32(m, obj + 0x08u);
        wr8(m, p + 0x22u, (uint8_t)(rd8(m, p + 0x22u) - 1u));
        wr8(m, p + 0x07u, (uint8_t)(rd8(m, p + 0x07u) | 0x01u));
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));

    switch (rd8(m, obj + 0x06u)) {
    case 0u:
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            Obj_StartBehaviour(m, obj, 0x000EDAB0u);
            wr8(m, obj + 0x06u, 1u);
        }
        break;
    case 1u:
        if (rd8(m, obj + 0x23u) == 0u) {
            if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
                break;
            Obj_AimAtPlayers(m, obj, k_rec);
            if (rd8(m, obj + 0x23u) == 0u)
                break;
        }
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) == 0u) {
            uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
            if ((int32_t)a1 >= 0) {
                wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
                wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x08u));
                wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
                wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x08u));
                wr16(m, a1 + 0x58u, rd16(m, obj + 0x42u));
                wr16(m, a1 + 0x10u, 0u);
                wr8(m, obj + 0x23u, 0x1Eu);
                wr8(m, obj + 0x06u, 2u);
            }
        }
        break;
    case 2u:
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) - 1u));
        if (rd8(m, obj + 0x23u) == 0u)
            wr8(m, obj + 0x06u, 1u);
        break;
    default:
        return -1;
    }

    Obj_StepScript(m, obj);
    Obj_0D0DE4(m, obj);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    return 0;
}
