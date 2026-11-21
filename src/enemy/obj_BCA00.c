#include "obj_BCA00.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BCA00 */
int Obj_BCA00_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
