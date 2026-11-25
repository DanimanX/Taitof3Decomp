#include "obj_FF4E0.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0FF4E0: tethered/hanging enemy tick */
int Obj_FF4E0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u)
        goto die_spawn;
    uint32_t parent = rd32(m, obj + 0x08u);
    if (rd8(m, parent + 0x06u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x0064u);
            uint32_t a1 = rd32(m, obj + 0x50u);
            uint32_t a2 = rd32(m, obj + 0x54u);
            wr32(m, a1 + 0x54u, a2);
            if ((int32_t)a2 >= 0)
                wr32(m, a2 + 0x50u, a1);
            if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
                Obj_StartDying(m, obj);
                return 0;
            }
            if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
                int16_t x2 = (int16_t)rd16(m, obj + 0x14u);
                if (x2 >= -0x0A && x2 <= 0x11C) {
                    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
                    Obj_ExplodeTick(m, obj);
                    return 0;
                }
            }
            wr32(m, obj + 0x00u, 0x000CEF44u);
            return 0;
        }
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u)
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x0064u);
    die_spawn:
        ;
        uint32_t c = Obj_Spawn(m, obj, 0x000DC43Eu, 3u, 2u, 0u);
        if ((int32_t)c >= 0) {
            wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 8u));
            wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 8u));
            wr16(m, c + 0x58u, 0u);
            wr16(m, c + 0x60u, 0u);
        }
        uint32_t p50 = rd32(m, obj + 0x50u);
        uint32_t p54 = rd32(m, obj + 0x54u);
        wr32(m, p50 + 0x54u, p54);
        if ((int32_t)p54 >= 0)
            wr32(m, p54 + 0x50u, p50);
        Obj_StartDying(m, obj);
        return 0;
    }
    if ((int16_t)rd16(m, obj + 0x18u) <= 0xD8) {
        int32_t d0 = (int32_t)rd32(m, obj + 0x54u);
        if (d0 < 0 || (uint16_t)(rd16(m, (uint32_t)d0 + 0x18u) - rd16(m, obj + 0x18u)) > 0x10u)
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
    }
    Obj_StepScript(m, obj);
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_BlinkColour(m, obj);
    wr32(m, obj + 0x00u, 0x000FF4E0u);
    return 0;
}
