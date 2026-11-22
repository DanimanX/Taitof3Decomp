#include "obj_C7EE6.h"

/* ROM 0x0C7EE6 */
int Obj_C7EE6_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    wr32(m, obj + 0x00u, t > 0 ? 0x000C7EE6u : 0x000C7EF4u);
    return 0;
}
