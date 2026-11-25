#include "obj_19163C.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"

/* ROM 0x19163C: trail of spawns drifting down-right (+4,+4) */
int Obj_19163C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((rd16(m, F3_A5 + 0x878u) & 0xFu) == 0u)
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x24));
    wr16(m, F3_A5 + 6u, (uint16_t)(rd16(m, F3_A5 + 6u) - 1u));
    if ((int16_t)rd16(m, F3_A5 + 6u) > 0) {
        wr32(m, obj, 0x19163Cu);
        return 0;
    }
    uint32_t d1 = ((uint32_t)rd16(m, obj + 0x14u) << 16) | rd16(m, obj + 0x18u);
    Obj_Spawn(m, obj, 0x1916BEu, 6, 1, d1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 4u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 4u));
    uint16_t c = (uint16_t)(rd16(m, obj + 0x22u) - 1u);
    wr16(m, obj + 0x22u, c);
    if ((int16_t)c > 0) {
        wr16(m, F3_A5 + 6u, (uint16_t)(Obj_RandRange(m, 5) + 5u));
        wr32(m, obj, 0x19163Cu);
        return 0;
    }
    /* move.w #2,(6,A0) leaves Z=0, so the beq to Obj_Free is never taken */
    wr16(m, obj + 6u, 2u);
    wr32(m, obj, 0x191688u);
    return 0;
}
