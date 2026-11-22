#include "obj_1287A4.h"
#include "../objects/obj_pool.h"

/* ROM 0x1287A4 */
int Obj_1287A4_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
