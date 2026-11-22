#include "obj_118E88.h"

/* ROM 0x0CBD86: clear animation / motion state */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0x118E88: arg = {x:10, y:10, flip:4, ...} (bitfields from the MSB) */
int Obj_118E88_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x14u, 0u);
    wr32(m, obj + 0x18u, 0u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)((int32_t)d0 >> 22));
    wr16(m, obj + 0x18u, (uint16_t)((int32_t)(d0 << 10) >> 22));
    if ((((int32_t)(d0 << 20)) >> 28) == 0) {
        wr16(m, obj + 0x42u, (uint16_t)-0x10);
        wr8(m, obj + 0x1Cu, 0u);
    } else {
        wr16(m, obj + 0x42u, 0x10u);
        wr8(m, obj + 0x1Cu, 1u);
    }
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x20u));
    wr32(m, obj + 0x00u, 0x00118EDEu);
    return 0;
}
