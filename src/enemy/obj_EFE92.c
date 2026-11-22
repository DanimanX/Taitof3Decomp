#include "obj_EFE92.h"
#include "../objects/obj_common.h"

/* ROM 0x0EFE92: test object state byte at +0x22; if zero, stop */
int Obj_EFE92_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x22u) == 0u)
        Obj_StartDying(m, obj);
    return 0;
}
