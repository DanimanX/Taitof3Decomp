#include "obj_E8DFE.h"
#include "obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../objects/objscript_CF1E4.h"
#include "../objects/objscript_CF4B8.h"

static void e8dfe_state1(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, obj + 0x07u) & 0x80u) != 0u)
        return;
    static const uint32_t pls[2] = { 0x0041B190u, 0x0041B21Au };
    for (int i = 0; i < 2; i++) {
        uint32_t p = pls[i];
        if ((int16_t)rd16(m, p + 0x4Cu) >= 0) {
            int16_t d0 = (int16_t)(rd16(m, obj + 0x14u) - rd16(m, p + 0x14u) - 0x0F);
            if (d0 < 0) d0 = (int16_t)(-d0);
            if ((uint16_t)d0 <= 0x46u) {
                wr32(m, obj + 0x44u, 0x000E92ACu);
                wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
                Obj_StartBehaviour(m, obj, 0x000E9316u);
                wr8(m, obj + 0x06u, 2u);
                return;
            }
        }
    }
}

static void e8dfe_state2(const f3mem_t *m, uint32_t obj)
{
    Obj_StepScript(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x20u) == 0u)
        return;
    if ((uint8_t)Obj_Next4Way(m) <= 0x08u) {
        (void)Obj_Spawn(m, obj, 0x000E953Au, 9u, 2u, 0u);
        wr32(m, obj + 0x44u, 0x000E92E8u);
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
        Obj_StartBehaviour(m, obj, 0x000E938Eu);
        wr8(m, obj + 0x06u, 3u);
    }
}

static void e8dfe_state3(const f3mem_t *m, uint32_t obj)
{
    Obj_StepScript(m, obj);
    if (rd8(m, obj + 0x20u) == 0u) {
        wr32(m, obj + 0x44u, 0x000E92FAu);
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x02u));
        Obj_StartBehaviour(m, obj, 0x000E9316u);
        wr8(m, obj + 0x06u, 4u);
    }
}

/* ROM 0x0E8DFE: radial-burst enemy (8 shots rotated by 45 degrees each) */
int Obj_E8DFE_Tick(const f3mem_t *m, uint32_t obj)
{
    const uint32_t k_aim_rec = 0x000E9250u;
    int goto_aim = 0;
    int goto_effect = 0;
    int goto_die = 0;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0xC8u);
            wr8(m, obj + 0x06u, 0x00u);
            Obj_0CEF0E(m, obj);
            return 0;
        }
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u) {
            uint32_t a1 = Obj_0CBDDE(m, obj);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0xC8u);
        }
        goto_aim = 1;
    } else if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
        goto_effect = 1;
    } else if (rd8(m, obj + 0x06u) == 0x05u) {
        goto_aim = 1;
    } else {
        wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
        wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));

        uint8_t f7 = rd8(m, obj + 0x07u);
        wr8(m, obj + 0x07u, (uint8_t)(f7 & (uint8_t)~0x02u));
        if ((f7 & 0x02u) != 0u) {
            if (ObjScript_CF1E4(m, obj) < 0)
                return -1;
        }
        if (rd8(m, obj + 0x20u) != 0u) {
            if (ObjScript_CF4B8(m, obj, k_aim_rec) < 0)
                return -1;
        }
        Obj_ArmOnScreen(m, obj);
        Obj_FlagOffscreen(m, obj);

        if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
            goto_die = 1;
        } else {
            wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
            wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
            switch (rd8(m, obj + 0x06u)) {
            case 1u: e8dfe_state1(m, obj); break;
            case 2u: e8dfe_state2(m, obj); break;
            case 3u: e8dfe_state3(m, obj); break;
            case 4u: Obj_StepScript(m, obj); break;
            default: return -1;
            }
            Obj_0D0DC2(m, obj);
            Obj_BlinkColour(m, obj);
            return 0;
        }
    }

    if (!goto_die && goto_aim) {
        Obj_AimAtPlayers(m, obj, k_aim_rec);
        if (rd8(m, obj + 0x23u) != 0u) {
            uint32_t d1 = rd32(m, obj + 0x14u) + 0x70000u;
            uint32_t d2 = rd32(m, obj + 0x18u) + 0x90000u;
            uint16_t d3 = rd16(m, obj + 0x42u);

            for (int i = 0; i < 8; i++) {
                uint32_t a1 = Obj_Spawn(m, obj, 0x000D9092u, 9u, 2u, 0u);
                if ((int32_t)a1 < 0) {
                    goto_die = 1;
                    break;
                }
                wr32(m, a1 + 0x14u, d1);
                wr32(m, a1 + 0x18u, d2);
                wr16(m, a1 + 0x10u, 0u);
                wr16(m, a1 + 0x58u, d3);

                d3 = (uint16_t)(d3 - 0x2Du);
                if ((int16_t)d3 < 0)
                    d3 = (uint16_t)(d3 + 0x168u);
            }
        }
        goto_effect = !goto_die;
    }

    if (!goto_die && goto_effect) {
        uint32_t a1 = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
        if ((int32_t)a1 >= 0) {
            wr32(m, a1 + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, a1 + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) + 0x0008u));
            wr32(m, a1 + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, a1 + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) + 0x0010u));
            wr16(m, a1 + 0x58u, 0u);
            wr16(m, a1 + 0x60u, 0u);
        }
    }

    wr8(m, obj + 0x06u, 0x00u);
    Obj_StartDying(m, obj);
    return 0;
}
