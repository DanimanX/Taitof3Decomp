#include "obj_EDBFE.h"
#include "../objects/obj_common.h"

/* ROM 0x0EDBFE */
int Obj_EDBFE_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0)
        Obj_StartDying(m, obj);
    return 0;
}
