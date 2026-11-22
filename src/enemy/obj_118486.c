#include "obj_118486.h"

/* ROM 0x118486: wait +6 frames, then 0x118494 */
int Obj_118486_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    wr32(m, obj + 0x00u, (int16_t)n > 0 ? 0x00118486u : 0x00118494u);
    return 0;
}
