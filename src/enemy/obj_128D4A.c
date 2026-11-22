#include "obj_128D4A.h"
#include "../objects/obj_pool.h"

/* ROM 0x128D4A */
int Obj_128D4A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
