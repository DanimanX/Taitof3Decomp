#include "obj_ED76E.h"
#include "../objects/obj_common.h"

/* ROM 0x0ED76E: if state byte at obj+0x22 is zero, start dying */
int Obj_ED76E_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        Obj_StartDying(m, obj);
    return 0;
}
