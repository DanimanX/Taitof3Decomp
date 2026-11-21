#include "obj_11C1DE.h"

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

/* ROM 0x11C1DE: clear 8 longs at 0x40B0F6, init */
int Obj_11C1DE_Tick(const f3mem_t *m, uint32_t obj)
{
    for (uint32_t i = 0; i < 8u; i++)
        wr32(m, 0x0040B0F6u + i * 4u, 0u);
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x00u, 0x0011C1FCu);
    return 0;
}
