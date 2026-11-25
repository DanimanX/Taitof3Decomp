#include "obj_10E178.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x10E178: enemy/effect init */
int Obj_10E178_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_SeedFromParams(m, obj, 0x0010E28Cu);
    wr8(m, obj + 0x07u, rd8(m, obj + 0x10u));
    uint32_t a1 = 0x0010E2C4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    wr16(m, obj + 0x3Au, 0u);
    if (rd8(m, obj + 0x07u) != 0u) {
        uint32_t parent = rd32(m, obj + 0x08u);
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, parent + 0x44u) - 0x10u));
    }
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr32(m, obj + 0x00u, 0x0010E1BCu);
    return 0;
}
