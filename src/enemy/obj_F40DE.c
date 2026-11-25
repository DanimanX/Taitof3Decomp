#include "obj_F40DE.h"
#include "../objects/obj_common.h"

/* ROM 0x0F40DE */
int Obj_F40DE_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        Obj_StartDying(m, obj);
    return 0;
}
