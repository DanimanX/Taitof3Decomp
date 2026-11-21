#include "obj_12EDA8.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x12EDA8 */
int Obj_12EDA8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x12EDA8: link a new child object behind A1 = its parent */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) + 1u));

    /* 0x12EDB6: clear the chain links and the two anchors */
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x48u, 0u);
    wr32(m, obj + 0x62u, 0u);
    wr32(m, obj + 0x66u, 0u);

    /* 0x12EDC8: walk the parent chain to its tail */
    uint32_t a2 = a1;
    while (rd32(m, a2 + 0x48u) != 0u)
        a2 = rd32(m, a2 + 0x48u);

    /* 0x12EDD6 */
    wr32(m, obj + 0x44u, a2);
    wr32(m, a2 + 0x48u, obj);
    wr32(m, obj + 0x62u, a2);
    wr32(m, a2 + 0x66u, obj);

    /* 0x12EDE6 */
    wr32(m, obj + 0x5Eu, 0x7FFFFFFFu);
    wr32(m, obj + 0x10u, 0x00028740u);
    wr8(m, obj + 0x1Du, 0u);                   /* sf (0x1d,A0) */
    wr16(m, obj + 0x34u, 0u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x10u));  /* bset.b #4 */
    wr16(m, obj + 0x30u, 0x0041u);
    wr16(m, obj + 0x32u, 0x0068u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr32(m, obj + 0x24u, 0u);

    /* 0x12EE1C */
    wr32(m, obj, 0x0012EE24u);
    return 0;
}
