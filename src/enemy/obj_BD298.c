#include "obj_BD298.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BD298: countdown that also drains a global 16.16 counter at
 * A5+0x422C by 0x600 per frame, then frees itself. */
int Obj_BD298_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x422Cu, rd32(m, F3_A5 + 0x422Cu) - 0x00000600u);
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BD298u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
