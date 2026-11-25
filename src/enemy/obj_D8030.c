#include "obj_D8030.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D8030: post-shot hold; bobs for 0x14 frames then transitions to 0x0D808C */
int Obj_D8030_Tick(const f3mem_t *m, uint32_t obj)
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
            wr32(m, obj + 0x00u, 0x000D8030u);
            return 0;
        }
    }

    wr16(m, obj + 0x06u, 0x1Eu);
    wr32(m, obj + 0x00u, 0x000D808Cu);
    return 0;
}
