#include "obj_C7F14.h"
#include "../objects/obj_pool.h"

/* ROM 0x0C7F14 */
int Obj_C7F14_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t t = rd16(m, a1); a1 += 2u;
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    wr32(m, obj + 0x36u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x00u, 0x000C7F3Eu);
    return 0;
}
