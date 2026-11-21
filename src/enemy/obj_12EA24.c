#include "obj_12EA24.h"
#include "../objects/obj_pool.h"

/* ROM 0x12EA24 */
int Obj_12EA24_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x32AAu, obj);
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x48u, 0u);
    wr32(m, obj + 0x62u, 0u);
    wr32(m, obj + 0x66u, 0u);
    wr16(m, obj + 0x22u, 0u);
    wr32(m, obj + 0x5Eu, 0xFFFFFFFFu);
    uint32_t a1 = 0x00131DE6u; /* lea (0x339E,PC) */
    for (int i = 0; i < 8; i++, a1 += 8u)
        Obj_Spawn(m, obj, 0x0012EB0Eu, 3u, 2u, a1);
    Obj_Spawn(m, obj, 0x0012EDA8u, 3u, 2u, 0u);
    wr16(m, obj + 0x06u, 0x0004u); /* Z=0: beq 0x12EAA4 never taken */
    wr32(m, obj + 0x00u, 0x0012EA9Eu);
    return 0;
}
