#include "obj_113644.h"
#include "../objects/obj_pool.h"

/* ROM 0x113644 */
int Obj_113644_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
