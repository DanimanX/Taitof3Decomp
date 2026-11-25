#include "obj_D653C.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D653C: enemy pre-fire anim state */
int Obj_D653C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
        uint32_t a1 = Obj_0CBDDE(m, obj);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        goto die_fx;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x20u) != 0u) {
    die_fx:
        Sound_PlayCmd(m, Sound_PreCmdHook(m, (int16_t)(uint16_t)rd8(m, obj + 0x65u)));
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, Obj_13E4D0(m, obj, 2u));
        wr32(m, obj + 0x00u, 0x000D6794u);
        return 0;
    }
    Obj_AnimFrameStep(m, obj, 0x000D67BCu);
    if ((int16_t)rd16(m, 0x000D67BCu + rd16(m, obj + 0x3Au)) <= 0) {
        wr16(m, obj + 0x06u, 3u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x5Eu, 0u);
        (void)Obj_Spawn(m, obj, 0x00118F4Eu, 0u, 1u, 0x14180000u);
        uint32_t child = Obj_Spawn(m, obj, 0x000D67F6u, 9u, 2u, 0u);
        if ((int32_t)child > 0) {
            wr32(m, child + 0x14u, rd32(m, obj + 0x14u));
            wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0Cu));
            wr32(m, child + 0x18u, rd32(m, obj + 0x18u));
            wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x10u));
        }
        wr32(m, obj + 0x00u, 0x000D6636u);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            wr32(m, obj + 0x10u, 0x000646C4u);
            Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    Obj_SubScrollVel(m, obj);
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x < -0x20 || x > 0x140 || y < -0x20 || y > 0xE8) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x00u, 0x000D653Cu);
    return 0;
}
