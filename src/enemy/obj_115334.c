#include "obj_115334.h"

/* ROM 0x115334 */
int Obj_115334_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x42u, rd16(m, a1));
    wr32(m, obj + 0x0Cu, rd32(m, a1 + 2u));
    wr16(m, obj + 0x2Cu, 0xFFF8u);
    wr16(m, obj + 0x30u, 0xFFF8u);
    wr16(m, obj + 0x2Eu, 7u);
    wr16(m, obj + 0x32u, 7u);
    a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x22u, rd16(m, a1 + 0x22u));
    wr16(m, a1 + 0x22u, (uint16_t)(rd16(m, a1 + 0x22u) - 1u));
    wr32(m, obj + 0x44u, 0u);
    wr32(m, obj + 0x48u, 0u);
    uint32_t a2 = rd32(m, a1 + 0x48u);
    if (a2 != 0u)
        wr32(m, a2 + 0x44u, obj);
    wr32(m, obj + 0x48u, a2);
    wr32(m, a1 + 0x48u, obj);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr32(m, obj + 0x24u, 0u);
    a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x20u, rd16(m, a1 + 0x20u));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr32(m, obj, 0x001153B8u);
    return 0;
}
