#include "obj_D6192.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../player/player_move.h"
#include "../video/sprite.h"

/* ROM 0x0D6192: enemy tick (bomb explode, draw, player proximity -> fire) */
int Obj_D6192_Tick(const f3mem_t *m, uint32_t obj)
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
        wr32(m, obj + 0x00u, 0x000D63FEu);
        return 0;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x11C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            wr32(m, obj + 0x10u, 0x0005D84Cu);
            Obj_RelayTimer(m, (uint32_t)rd16(m, obj + 0x4Cu));
            Obj_ExplodeTick(m, obj);
            return 0;
        }
    }
    wr32(m, obj + 0x10u, 0x0005D830u);
    Obj_SubScrollVel(m, obj);
    wr32(m, rd32(m, F3_A5 + 0x08FEu), obj);
    wr32(m, F3_A5 + 0x08FEu, rd32(m, F3_A5 + 0x08FEu) + 4u);
    wr32(m, rd32(m, F3_A5 + 0x0906u), obj);
    wr32(m, F3_A5 + 0x0906u, rd32(m, F3_A5 + 0x0906u) + 4u);
    wr8(m, obj + 0x35u, 0u);
    Obj_BlinkColour(m, obj);
    Obj_DrawSprite(m, obj);
    uint32_t tbl = F3_A5 + 0x1D2Cu;
    int16_t cnt = (int16_t)rd16(m, F3_A5 + 0x1D34u);
    while (--cnt >= 0) {
        uint32_t p = rd32(m, tbl); tbl += 4u;
        int16_t dx = (int16_t)(rd16(m, p + 0x14u) - rd16(m, obj + 0x14u));
        if (dx >= -0x8C && dx <= 0x8C) {
            (void)Obj_Spawn(m, obj, 0x00118F4Eu, 0u, 1u, 0x180C0002u);
            wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
            wr32(m, obj + 0x00u, 0x000D6242u);
            return 0;
        }
    }
    wr32(m, obj + 0x00u, 0x000D6192u);
    return 0;
}
