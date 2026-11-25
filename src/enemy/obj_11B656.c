#include "obj_11B656.h"
#include "../objects/obj_pool.h"

/* ROM 0x11B656 */
int Obj_11B656_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t p = rd32(m, F3_A5 + 0x1D28u);
    wr32(m, obj + 0x0Cu, p);
    if (p == 0u) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj, 0x0011B668u);
    return 0;
}
