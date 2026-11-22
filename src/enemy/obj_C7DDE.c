#include "obj_C7DDE.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C7DDE */
int Obj_C7DDE_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, F3_A5 + 0x1D4Au) + 1u);
    wr16(m, F3_A5 + 0x1D4Au, (uint16_t)t);
    if (t < 0x0F) {
        wr32(m, obj + 0x00u, 0x000C7DDEu);
        return 0;
    }
    Obj_Spawn(m, obj, 0x000C7F14u, 9u, 2u, 0x000C7FC2u);
    wr32(m, obj + 0x00u, 0x000C7E10u);
    return 0;
}
