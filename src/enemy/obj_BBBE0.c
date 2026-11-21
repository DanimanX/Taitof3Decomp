#include "obj_BBBE0.h"

/* ROM 0x0CBD86 */
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

/* ROM 0x0BBBE0 */
int Obj_BBBE0_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    wr16(m, obj + 0x60u, 0x003Fu);
    wr32(m, obj + 0x00u, 0x000BBBF4u);
    return 0;
}
