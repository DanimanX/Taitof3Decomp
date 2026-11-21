#include "obj_BBAE4.h"
#include "../objects/obj_pool.h"

/* ROM 0x0BBAE4 */
int Obj_BBAE4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBAE4 */
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, cnt);
    if (cnt > 0u) {
        /* 0x0BBADC */
        wr32(m, obj, 0x000BBAE4u);
        return 0;
    }

    /* 0x0BBAEA: spawn 0xBBBE0 (group 0..0) */
    Obj_Spawn(m, obj, 0x000BBBE0u, 0u, 1u, 0u);
    wr16(m, obj + 0x06u, 0x0050u);          /* move.w #0x50,(0x6,A0) */
    if ((int16_t)rd16(m, obj + 0x06u) != 0) {
        wr32(m, obj, 0x000BBB18u);          /* 0x0BBB10 */
        return 0;
    }

    /* 0x0BBB1E: spawn 0xBBD24, 0xBBD6E, 0xBBDE0, 0xBBE2A, 0xBBE9A, 0xBC1CE */
    Obj_Spawn(m, obj, 0x000BBD24u, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBD6Eu, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBDE0u, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BBD6Eu + 0xBCu, 0u, 1u, 0u); /* 0xBBE2A */
    Obj_Spawn(m, obj, 0x000BBE9Au, 0u, 1u, 0u);
    Obj_Spawn(m, obj, 0x000BC1CEu, 0u, 1u, 0u);
    Obj_Free(m, obj);                       /* 0x0BBBD2 */
    return 0;
}
