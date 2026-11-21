#include "obj_BBB18.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BBB18 */
int Obj_BBB18_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBB18 */
    uint16_t d0 = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, d0);
    if ((int16_t)d0 > 0) {
        wr32(m, obj, 0x000BBB18u);          /* 0x0BBB10 */
        return 0;
    }

    /* 0x0BBB1E: spawn 6 objects (group 0..0) */
    Obj_Spawn(m, obj, 0x000BBD24u, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBD6Eu, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBDE0u, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBE2Au, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBE9Au, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BC1CEu, 0u, 1u, 0u);

    /* 0x0BBBD2 */
    Obj_Free(m, obj);
    return 0;
}
