#include "obj_F0D7E.h"
#include "../objects/obj_common.h"

/* ROM 0x0F0D7E */
int Obj_F0D7E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        Obj_StartDying(m, obj);
    return 0;
}
