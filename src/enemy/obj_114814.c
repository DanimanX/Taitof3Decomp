#include "obj_114814.h"
#include "../objects/obj_pool.h"

/* ROM 0x114814 */
int Obj_114814_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
