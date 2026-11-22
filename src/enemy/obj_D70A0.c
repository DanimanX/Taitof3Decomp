#include "obj_D70A0.h"
#include "../objects/obj_pool.h"

/* ROM 0x0D70A0 */
int Obj_D70A0_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
