#include "obj_BD256.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BD256 */
int Obj_BD256_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x4238u, rd32(m, F3_A5 + 0x4238u) - 0xC0u);
    wr32(m, F3_A5 + 0x4240u, rd32(m, F3_A5 + 0x4240u) - 0x60u);
    wr32(m, F3_A5 + 0x423Cu, rd32(m, F3_A5 + 0x423Cu) - 0x30u);
    uint16_t t = (uint16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, t);
    if ((int16_t)t >= 0) {
        wr32(m, obj + 0x00u, 0x000BD256u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
