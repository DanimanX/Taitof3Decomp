#include "obj_C8218.h"

/* ROM 0x0C8218 */
int Obj_C8218_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000C8218u);
        return 0;
    }
    wr16(m, obj + 0x44u, 0u);
    wr16(m, obj + 0x46u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x00u, 0x000C823Eu);
    return 0;
}
