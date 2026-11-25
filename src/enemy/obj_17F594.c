#include "obj_17F594.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x17F594: inherit parent offset pos, bounds check, play sfx 0x23, arm 0x17F5DA */
int Obj_17F594_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InheritOffsetPos(m, obj);
    int16_t x = (int16_t)rd16(m, obj + 0x50u);
    int16_t y = (int16_t)rd16(m, obj + 0x54u);
    if (x < -0x10 || x > 0x14F || y < -0x10 || y > 0xF7) {
        Obj_Free(m, obj);
        return 0;
    }
    wr16(m, obj + 0x64u, 0x003Cu);
    Sound_PlayCmd(m, Sound_PreCmdHook(m, 0x0023));
    wr32(m, obj + 0x00u, 0x0017F5DAu);
    return 0;
}
