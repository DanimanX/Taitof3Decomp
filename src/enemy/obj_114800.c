#include "obj_114800.h"

/* ROM 0x114800: wait +0x6 ticks, then set bit 5 of +0x50 */
int Obj_114800_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x00114800u);
        return 0;
    }
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
    wr32(m, obj + 0x00u, 0x00114814u);
    return 0;
}
