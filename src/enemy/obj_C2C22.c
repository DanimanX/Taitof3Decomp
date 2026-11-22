#include "obj_C2C22.h"

/* ROM 0x0C2C22 */
int Obj_C2C22_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x000C2C22u);
        return 0;
    }
    wr16(m, obj + 0x44u, 0x007Fu);
    wr32(m, obj + 0x00u, 0x000C2C36u);
    return 0;
}
