#include "obj_EEB76.h"
#include "../objects/obj_common.h"

/* ROM 0x0EEB76: wait until the child count at +0x22 drops to 0, then die */
int Obj_EEB76_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        Obj_StartDying(m, obj);
    return 0;
}
