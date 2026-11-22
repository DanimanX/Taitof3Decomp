#include "obj_114F38.h"
#include "../objects/obj_pool.h"

/* ROM 0x114F38 */
int Obj_114F38_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
