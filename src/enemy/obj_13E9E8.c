#include "obj_13E9E8.h"
#include "../objects/obj_pool.h"

/* ROM 0x13E9E8: pick unique random slot 0..15, spawn 0x13EA54, arm 0x13EA38 */
int Obj_13E9E8_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x0Eu) << 2));
    uint32_t d1 = 0u;
    uint32_t rng;
    uint16_t d0;
    for (;;) {
        rng = Rng_NextMem(m);
        d0 = (uint16_t)(rng & 0x000Fu);
        if ((d1 & (1u << d0)) == 0u) {
            d1 |= (1u << d0);
            break;
        }
    }
    wr32(m, obj + 0x3Au, d1);
    (void)Obj_Spawn(m, obj, 0x0013EA54u, 9u, 2u, (rng & 0xFFFF0000u) | d0);
    wr16(m, obj + 0x06u, 0x0010u);
    wr32(m, obj + 0x00u, 0x0013EA38u);
    return 0;
}
