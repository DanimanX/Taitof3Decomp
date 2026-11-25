#include "obj_116B0C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x116B0C: init linked segment object and spawn 3 children (0x116C48) */
int Obj_116B0C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    wr16(m, obj + 0x28u, 1u);
    wr16(m, obj + 0x2Cu, (uint16_t)-4);
    wr16(m, obj + 0x30u, (uint16_t)-4);
    wr16(m, obj + 0x2Eu, 4u);
    wr16(m, obj + 0x32u, 4u);
    wr16(m, obj + 0x22u, 1u);
    uint32_t d0_arg = obj;
    for (int i = 0; i < 3; i++)
        d0_arg = Obj_Spawn(m, obj, 0x00116C48u, 3u, 2u, d0_arg);
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t d0 = rd32(m, a1 + 0x48u);
    if (d0 == 0u) {
        wr32(m, a1 + 0x44u, obj);
        d0 = a1;
    }
    wr32(m, a1 + 0x48u, obj);
    uint32_t a2 = d0;
    wr32(m, a2 + 0x48u, obj);
    wr32(m, obj + 0x44u, a2);
    wr32(m, obj + 0x50u, 0u);
    wr32(m, obj + 0x00u, 0x00116B86u);
    return 0;
}
