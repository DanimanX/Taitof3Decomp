#include "obj_BB83E.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BB83E */
int Obj_BB83E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
