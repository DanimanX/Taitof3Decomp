#include "obj_BC3FA.h"

/* ROM 0x0CBD86: clear position/animation state */
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

/* ROM 0x0BC3FA: init from the table at 0x0BC42C (common body 0x0BC47C) */
int Obj_BC3FA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x000BC42Cu;
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x44u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x0086u);
    wr32(m, obj + 0x00u, 0x000BC4A8u);
    return 0;
}
