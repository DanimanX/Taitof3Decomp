#include "obj_11699A.h"
#include "../objects/obj_pool.h"

/* ROM 0x11699A */
int Obj_11699A_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t c = (int16_t)(rd16(m, obj + 0x06u) - 1);
    wr16(m, obj + 0x06u, (uint16_t)c);
    if (c > 0) {
        wr32(m, obj, 0x0011699Au);
        return 0;
    }
    wr8(m, obj + 0x50u, (uint8_t)(rd8(m, obj + 0x50u) | 0x20u));
    Obj_Free(m, rd32(m, obj + 0x08u));
    Obj_Free(m, rd32(m, obj + 0x0Cu));
    wr32(m, obj, 0x001169CAu);
    return 0;
}
