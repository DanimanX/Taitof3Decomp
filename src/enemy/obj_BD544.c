#include "obj_BD544.h"

/* ROM 0x0BD544 */
int Obj_BD544_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x60u, 0u);
    wr32(m, obj + 0x00u, 0x000BD550u);
    return 0;
}
