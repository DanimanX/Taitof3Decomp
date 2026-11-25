#include "obj_11BFAC.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x11BFAC: boss/ship charge-up + explosion burst */
int Obj_11BFAC_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_AnimFrameStep(m, obj, 0x0011C064u);
    if ((int16_t)rd16(m, 0x0011C064u + rd16(m, obj + 0x3Au)) > 0) {
        Obj_MoveByAngle(m, obj, 0u);
        Obj_DrawSprite(m, obj);
        wr32(m, obj + 0x00u, 0x0011BFACu);
        return 0;
    }
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    uint32_t d1 = 2u | (((d0 >> 16) & 0xFFu) << 24) | ((d0 & 0xFFu) << 16);
    wr32(m, obj + 0x0Cu, d1);
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    uint32_t parent = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
    if ((Rng_NextMem(m) & 0x1Fu) == 0u) {
        (void)Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d1);
        if ((Rng_NextMem(m) & 0x0Fu) == 0u)
            Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0033));
    }
    wr32(m, obj + 0x00u, 0x0011C054u);
    return 0;
}
