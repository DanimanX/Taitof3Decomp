#include "obj_1915E0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../audio/sound_cmd.h"

/* ROM 0x19169C: every 16th frame play sound 0x24 */
static void sub_19169C(const f3mem_t *m)
{
    if ((rd16(m, F3_A5 + 0x878u) & 0xFu) == 0u)
        Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x24));
}

/* ROM 0x1915E0: trail of spawns drifting up-left (-4,-4) */
int Obj_1915E0_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_19169C(m);
    wr16(m, F3_A5 + 6u, (uint16_t)(rd16(m, F3_A5 + 6u) - 1u));
    if ((int16_t)rd16(m, F3_A5 + 6u) > 0) {
        wr32(m, obj, 0x1915E0u);
        return 0;
    }
    uint32_t d1 = ((uint32_t)rd16(m, obj + 0x14u) << 16) | rd16(m, obj + 0x18u);
    Obj_Spawn(m, obj, 0x1916BEu, 6, 1, d1);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 4u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 4u));
    uint16_t c = (uint16_t)(rd16(m, obj + 0x22u) - 1u);
    wr16(m, obj + 0x22u, c);
    if ((int16_t)c > 0) {
        wr16(m, F3_A5 + 6u, (uint16_t)(Obj_RandRange(m, 5) + 5u));
        wr32(m, obj, 0x1915E0u);
        return 0;
    }
    wr16(m, obj + 0x22u, 0x10u);
    wr16(m, F3_A5 + 6u, (uint16_t)(Obj_RandRange(m, 5) + 5u));
    wr32(m, obj, 0x19163Cu);
    return 0;
}
