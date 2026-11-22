#include "obj_C7DBA.h"

/* ROM 0x0C7DBA */
int Obj_C7DBA_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0x002Cu);
    wr32(m, obj + 0x00u, 0x000C7DCAu);
    return 0;
}
