#include "obj_D7F0E.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D7F0E: bobbing pose sequence; steps +0x62/+0x5C, then hands over to 0x0D7F52 */
int Obj_D7F0E_Tick(const f3mem_t *m, uint32_t obj)
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

    wr16(m, obj + 0x5Cu, (uint16_t)rd8(m, obj + 0x62u));
    Obj_0D8378(m, obj);
    Obj_0D825E(m, obj);
    Obj_0D8294(m, obj);

    wr16(m, obj + 0x62u, (uint16_t)(rd16(m, obj + 0x62u) + 0x40u));
    if ((int8_t)rd8(m, obj + 0x62u) < 6) {
        wr32(m, obj + 0x00u, 0x000D7F0Eu);
        return 0;
    }

    wr16(m, obj + 0x5Cu, 5u);
    wr32(m, obj + 0x00u, 0x000D7F52u);
    return 0;
}
