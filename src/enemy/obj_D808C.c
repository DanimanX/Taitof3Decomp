#include "obj_D808C.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0D808C: telegraph & pick dash angle into +0x5E, then hand over to 0x0D8164 */
int Obj_D808C_Tick(const f3mem_t *m, uint32_t obj)
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

    wr16(m, obj + 0x5Cu, ((rd16(m, obj + 0x06u) & 3u) != 0u) ? 6u : 7u);
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
            wr32(m, obj + 0x00u, 0x000D808Cu);
            return 0;
        }
    }

    wr16(m, obj + 0x5Cu, 8u);
    wr16(m, obj + 0x06u, 0x3Cu);

    {
        /* cmp2.w (0x0D83DE)/(0x0D83E2): x in [0x3C, 0x104], y in [0x3C, 0xAA] */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        uint16_t d0;
        if (x < 0x3C || x > 0x104 || y < 0x3C || y > 0xAA) {
            uint16_t aim = Obj_Atan2(m, (uint16_t)(0xA0 - x), (uint16_t)(0x74 - y));
            uint8_t jitter = (uint8_t)((Rng_NextMem(m) & 0x3Fu) - 0x20u);
            d0 = (uint16_t)((uint8_t)(aim + jitter));
        } else {
            d0 = (uint16_t)(Rng_NextMem(m) & 0xFFu);
        }
        wr16(m, obj + 0x5Eu, d0);
    }

    wr32(m, obj + 0x00u, 0x000D8164u);
    return 0;
}
