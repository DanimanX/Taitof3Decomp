#include "obj_E262A.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "obj_E8B72.h"

/* ROM 0x0E262A: ground/turret base tick */
int Obj_E262A_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Obj_RelayTimer(m, 0x00C8u);
            wr8(m, obj + 0x06u, 0u);
            wr32(m, obj + 0x10u, 0x000643D4u);
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 6u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 9u));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        if (rd8(m, obj + 0x35u) == 1u || rd8(m, obj + 0x35u) == 2u)
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00C8u);
        goto die_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_spawn:
        ;
        uint32_t c = Obj_Spawn(m, obj, 0x000DC456u, 3u, 2u, 0u);
        if ((int32_t)c >= 0) {
            wr32(m, c + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, c + 0x14u, (uint16_t)(rd16(m, c + 0x14u) + 8u));
            wr32(m, c + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, c + 0x18u, (uint16_t)(rd16(m, c + 0x18u) + 8u));
            wr16(m, c + 0x58u, 0u);
            wr16(m, c + 0x60u, 0u);
        }
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) - rd32(m, F3_A5 + 0x4224u));
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - rd32(m, F3_A5 + 0x4228u));
    Obj_FlagOffscreen(m, obj);
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        wr8(m, obj + 0x06u, 0u);
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    switch (rd8(m, obj + 0x06u)) {
    case 1u:
        Obj_ArmOnScreen(m, obj);
        if ((rd8(m, obj + 0x07u) & 0x80u) == 0u) {
            wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));
            wr8(m, obj + 0x06u, 2u);
        }
        break;
    case 2u: {
        uint32_t a1 = 0x000E27ACu;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
        wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
        break;
    }
    case 3u: {
        uint32_t a1 = 0x000E27B6u;
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
        wr32(m, obj + 0x10u, rd32(m, a1 + 2u));
        wr32(m, obj + 0x68u, rd32(m, a1 + 6u));
        wr8(m, obj + 0x06u, 2u);
        break;
    }
    default:
        return -1;
    }
    Obj_0D0DC2(m, obj);
    Obj_BlinkColour(m, obj);
    wr32(m, obj + 0x00u, 0x000E262Au);
    return 0;
}
