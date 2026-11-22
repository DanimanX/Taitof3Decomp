#include "obj_CEF54.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CEF54 */
int Obj_CEF54_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Free(m, obj);
    return 0;
}
