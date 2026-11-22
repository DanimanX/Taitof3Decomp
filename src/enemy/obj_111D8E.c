#include "obj_111D8E.h"
#include "../objects/obj_pool.h"

/* ROM 0x111D8E: retire the object (bomb-burst chain target of 0x111D18) */
int Obj_111D8E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
