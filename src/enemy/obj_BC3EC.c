#include "obj_BC3EC.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BC3EC: free the object */
int Obj_BC3EC_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
