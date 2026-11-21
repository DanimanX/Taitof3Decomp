#include "obj_BB860.h"

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

/* ROM 0x0BB860: init from the table at 0x0BB8B6 (common body 0x0BB90C) */
int Obj_BB860_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x000BB8B6u;
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x48u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x4Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x50u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x54u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Fu, rd8(m, a1)); a1 += 1u;
    wr8(m, obj + 0x1Eu, rd8(m, a1)); a1 += 1u;
    wr16(m, obj + 0x58u, rd16(m, a1));
    wr16(m, obj + 0x5Cu, rd16(m, obj + 0x58u));
    wr16(m, obj + 0x60u, 0x00B3u);
    wr32(m, obj + 0x00u, 0x000BB952u);
    return 0;
}
