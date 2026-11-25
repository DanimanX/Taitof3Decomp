#include "obj_D7EAE.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D7EAE: entry slide; advances +0x54 toward centre, then hands over to 0x0D7F0E */
int Obj_D7EAE_Tick(const f3mem_t *m, uint32_t obj)
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
        int16_t d0 = (int16_t)rd16(m, obj + 0x5Eu);
        int16_t d1 = (int16_t)(rd16(m, obj + 0x06u) >> 2);
        if (d0 > d1) {
            d0 = d1;
            wr16(m, obj + 0x5Cu, 1u);
        }
        if (rd8(m, obj + 0x64u) == 0u)
            wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + (uint16_t)d0));
        else
            wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) - (uint16_t)d0));
    }

    Obj_0D825E(m, obj);
    Obj_0D8294(m, obj);

    {
        int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)t);
        if (t > 0) {
            wr32(m, obj + 0x00u, 0x000D7EAEu);
            return 0;
        }
    }

    wr16(m, obj + 0x62u, 0x0200u);
    wr16(m, obj + 0x06u, 0x00A0u);
    wr32(m, obj + 0x00u, 0x000D7F0Eu);
    return 0;
}
