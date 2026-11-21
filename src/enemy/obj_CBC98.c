#include "obj_CBC98.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CBC98: free the object */
int Obj_CBC98_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
