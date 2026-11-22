#include "obj_1192A0.h"

/* ROM 0x1192A0: wait +6 frames, then 0x1192AE */
int Obj_1192A0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t n = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, n);
    wr32(m, obj + 0x00u, (int16_t)n > 0 ? 0x001192A0u : 0x001192AEu);
    return 0;
}
