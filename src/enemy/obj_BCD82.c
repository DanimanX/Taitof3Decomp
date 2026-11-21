#include "obj_BCD82.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BCD82: immediately frees the object. */
int Obj_BCD82_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
