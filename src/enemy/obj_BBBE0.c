#include "obj_BBBE0.h"
#include "../objects/obj_common.h"

/* ROM 0x0BBBE0 */
int Obj_BBBE0_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr16(m, obj + 0x60u, 0x003Fu);
    wr32(m, obj + 0x00u, 0x000BBBF4u);
    return 0;
}
