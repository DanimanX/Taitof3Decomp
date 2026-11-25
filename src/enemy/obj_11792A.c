#include "obj_11792A.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x11792A: init child object, play sound 0x29, arm 0x117980 */
int Obj_11792A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_113652(m, obj);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr32(m, obj + 0x3Au, 0u);
    wr32(m, obj + 0x3Eu, 0u);
    wr16(m, obj + 0x30u, 2u);
    wr16(m, obj + 0x32u, 0x000Du);
    wr8(m, obj + 0x1Du, 0x38u);
    wr16(m, obj + 0x42u, 0u);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0029));
    wr16(m, obj + 0x06u, 0x00B4u);
    wr32(m, obj + 0x00u, 0x00117980u);
    return 0;
}
