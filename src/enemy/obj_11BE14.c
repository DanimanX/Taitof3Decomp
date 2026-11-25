#include "obj_11BE14.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x11BE14: orbiting satellite/pod tick */
int Obj_11BE14_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0 || (rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0035));
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, Obj_13E4D0(m, obj, 1u));
        wr32(m, obj + 0x00u, 0x0011BEF6u);
        return 0;
    }
    Obj_Animate(m, obj, 0x0011BF2Au);
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            uint32_t parent = rd32(m, obj + 0x08u);
            int16_t v = (int16_t)(rd16(m, parent + 0x22u) - 1u);
            wr16(m, parent + 0x22u, (uint16_t)v);
            if (v <= 0)
                wr16(m, parent + 0x22u, (uint16_t)(v - 1));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);

    uint32_t parent = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, rd16(m, parent + 0x14u));
    wr16(m, obj + 0x18u, rd16(m, parent + 0x18u));
    uint8_t d0 = (uint8_t)((rd8(m, obj + 0x0Fu) << 5) + rd8(m, parent + 0x3Au));
    uint16_t d1 = rd16(m, obj + 0x06u);
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) + 1u));
    int16_t d2 = (int16_t)rd16(m, obj + 0x42u);
    int16_t d3 = (int16_t)((((int32_t)d2 * (int16_t)rd16(m, 0x00027E40u + d1 * 2u)) << 2) >> 16);
    int32_t dx = (int32_t)d2 * (int16_t)rd16(m, 0x00027E40u + 0x80u + d0 * 2u);
    int32_t dy = (int32_t)d3 * (int16_t)rd16(m, 0x00027E40u + d0 * 2u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)dy);
    Obj_11C32C(m, obj);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0011BE14u);
    return 0;
}
