#include "obj_D7F52.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D7F52: zoom-in approach; decrements +0x60 until 0, then hands over to 0x0D7F8E */
int Obj_D7F52_Tick(const f3mem_t *m, uint32_t obj)
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

    Obj_0D825E(m, obj);
    Obj_0D8294(m, obj);

    {
        int16_t s = (int16_t)(rd16(m, obj + 0x60u) - 4u);
        wr16(m, obj + 0x60u, (uint16_t)s);
        if (s > 0) {
            wr32(m, obj + 0x00u, 0x000D7F52u);
            return 0;
        }
    }

    wr16(m, obj + 0x62u, 0u);
    wr16(m, obj + 0x60u, 0u);
    wr16(m, obj + 0x5Cu, 4u);
    wr16(m, obj + 0x06u, 0x12u);
    wr32(m, obj + 0x00u, 0x000D7F8Eu);
    return 0;
}
