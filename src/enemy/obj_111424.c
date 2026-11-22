#include "obj_111424.h"
#include "../objects/obj_pool.h"

/* ROM 0x111424 */
int Obj_111424_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
