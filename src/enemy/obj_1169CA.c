#include "obj_1169CA.h"
#include "../objects/obj_pool.h"

/* ROM 0x1169CA */
int Obj_1169CA_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
