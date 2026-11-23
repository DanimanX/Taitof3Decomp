#include "obj_BCE8E.h"
#include "../objects/obj_common.h"

/* ROM 0x0BCE8E */
int Obj_BCE8E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x10u, 0x0003B654u);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x0Cu));
    wr16(m, obj + 0x60u, 0x000Bu);
    wr32(m, obj + 0x00u, 0x000BCEB0u);
    return 0;
}
