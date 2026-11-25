#include "obj_D8164.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D8164: dash along +0x5E with triangle speed profile; spawns 0x0D853C and loops to 0x0D8030 */
int Obj_D8164_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_fx;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_fx:
        Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)rd8(m, obj + 0x65u)));
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, 2u);
        wr32(m, obj + 0x00u, 0x000D823Eu);
        return 0;
    }

    {
        uint16_t dir = rd16(m, obj + 0x5Eu);
        int16_t d1 = (int16_t)rd16(m, obj + 0x06u) - 0x1E;
        if (d1 < 0)
            d1 = (int16_t)-d1;
        d1 = (int16_t)(0x1E - d1);
        int32_t dx = ((int32_t)d1 * (int16_t)rd16(m, 0x027E40u + 0x80u + (uint32_t)dir * 2u)) >> 2;
        int32_t dy = ((int32_t)d1 * (int16_t)rd16(m, 0x027E40u + (uint32_t)dir * 2u)) >> 2;
        wr32(m, obj + 0x54u, (uint32_t)((int32_t)rd32(m, obj + 0x54u) + dx));
        wr32(m, obj + 0x58u, (uint32_t)((int32_t)rd32(m, obj + 0x58u) + dy));
    }

    Obj_0D8378(m, obj);
    Obj_0D825E(m, obj);
    Obj_0D8294(m, obj);

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            wr32(m, obj + 0x10u, 0x0002BADCu);
            Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }

    Obj_0D0DC2(m, obj);
    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {
            wr32(m, obj + 0x00u, 0x000D8164u);
            return 0;
        }
    }

    (void)Obj_Spawn(m, obj, 0x000D853Cu, 5u, 2u, 0u);
    wr16(m, obj + 0x5Cu, 6u);
    wr16(m, obj + 0x06u, 0x14u);
    wr32(m, obj + 0x00u, 0x000D8030u);
    return 0;
}
