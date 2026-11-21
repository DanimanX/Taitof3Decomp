#include "obj_1180F8.h"
#include "../objects/obj_pool.h"

/* ROM 0x1180F8 */
int Obj_1180F8_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
