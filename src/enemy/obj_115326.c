#include "obj_115326.h"
#include "../objects/obj_pool.h"

/* ROM 0x115326 */
int Obj_115326_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
