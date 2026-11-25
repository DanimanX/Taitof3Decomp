#include "obj_13EA38.h"
#include "../objects/obj_pool.h"

/* ROM 0x13EA38: countdown +0x06, spawn up to 10 unique 0x13EA54 children, else free */
int Obj_13EA38_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0013EA38u);
        return 0;
    }
    if ((int16_t)rd16(m, obj + 0x22u) <= 9) {
        uint32_t d1 = rd32(m, obj + 0x3Au);
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
    Obj_Free(m, obj);
    return 0;
}
