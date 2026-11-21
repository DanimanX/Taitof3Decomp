#include "obj_BD354.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BD354: accelerate the scroll layers for +0x60 frames, then free */
int Obj_BD354_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x4230u, rd32(m, F3_A5 + 0x4230u) + 0x2A0u);
    wr32(m, F3_A5 + 0x4238u, rd32(m, F3_A5 + 0x4238u) + 0x2A0u);
    wr32(m, F3_A5 + 0x4240u, rd32(m, F3_A5 + 0x4240u) + 0x2A0u);
    wr32(m, F3_A5 + 0x422Cu, rd32(m, F3_A5 + 0x422Cu) + 0x7D0u);
    wr32(m, F3_A5 + 0x4234u, rd32(m, F3_A5 + 0x4234u) + 0x7D0u);
    wr32(m, F3_A5 + 0x423Cu, rd32(m, F3_A5 + 0x423Cu) + 0x7D0u);
    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BD354u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
