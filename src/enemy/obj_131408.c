#include "obj_131408.h"

/* ROM 0x131408: clear field at 0x3A and set the next function pointer */
int Obj_131408_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Au, 0u);
    wr32(m, obj, 0x00131414u);
    return 0;
}
