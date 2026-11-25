#include "obj_186326.h"
#include "../objects/obj_pool.h"

/* ROM 0x186326: articulated boss body init. Spawns 5 linked segments (0x186406,
 * group 3 of 2, args 4..0) in a doubly-linked +0x62/+0x66 ring with obj, sets
 * initial offsets/joint state, and installs 0x186388. */
int Obj_186326_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a3 = obj;
    uint32_t a4 = obj;
    uint32_t d0 = 0u;

    for (int16_t d7 = 4; d7 >= 0; d7--) {
        d0 = Obj_Spawn(m, obj, 0x00186406u, 3u, 2u, 0xFFFF0000u | (uint16_t)d7);
        wr32(m, a3 + 0x62u, d0);
        a4 = d0;
        wr32(m, a4 + 0x66u, a3);
        a3 = a4;
    }

    wr32(m, obj + 0x66u, d0);
    wr32(m, a4 + 0x62u, obj);
    wr16(m, obj + 0x16u, 0x003Cu);
    wr16(m, obj + 0x1Au, 0x0011u);
    wr16(m, obj + 0x42u, 0x4000u);
    wr16(m, obj + 0x5Cu, 0u);
    wr16(m, obj + 0x5Eu, 0u);
    wr16(m, obj + 0x60u, 0u);
    wr32(m, obj + 0x00u, 0x00186388u);
    return 0;
}
