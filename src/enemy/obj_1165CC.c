#include "obj_1165CC.h"
#include "../objects/obj_pool.h"

/* ROM 0x1165CC: countdown +0x06, unlink, spawn 8x 0x116DD2, arm 0x1166EA */
int Obj_1165CC_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x001165CCu);
        return 0;
    }
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint32_t a2 = rd32(m, obj + 0x48u);
    wr32(m, a2 + 0x48u, a1);
    wr32(m, a1 + 0x44u, a2);
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x48u, 0u);
    for (uint32_t arg = 0x1000u; arg <= 0xF000u; arg += 0x2000u)
        (void)Obj_Spawn(m, obj, 0x00116DD2u, 3u, 2u, arg);
    wr16(m, obj + 0x06u, 2u);
    wr32(m, obj + 0x00u, 0x001166EAu);
    return 0;
}
