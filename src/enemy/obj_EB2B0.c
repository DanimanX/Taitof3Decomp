#include "obj_EB2B0.h"
#include "../objects/obj_common.h"

/* ROM 0x0EB2B0: wait until the child count at +0x22 drops to 0, then die */
int Obj_EB2B0_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        Obj_StartDying(m, obj);
    return 0;
}
