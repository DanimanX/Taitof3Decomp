#include "obj_C7EF4.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C7EF4 */
int Obj_C7EF4_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, F3_A5 + 0x1D4Au) - 1u);
    wr16(m, F3_A5 + 0x1D4Au, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000C7EF4u);
        return 0;
    }
    wr8(m, F3_A5 + 0x1D36u, (uint8_t)(rd8(m, F3_A5 + 0x1D36u) & ~0x04u));
    Obj_Free(m, obj);
    return 0;
}
