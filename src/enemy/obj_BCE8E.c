#include "obj_BCE8E.h"

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

/* ROM 0x0BCE8E */
int Obj_BCE8E_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x10u, 0x0003B654u);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x0Cu));
    wr16(m, obj + 0x60u, 0x000Bu);
    wr32(m, obj + 0x00u, 0x000BCEB0u);
    return 0;
}
