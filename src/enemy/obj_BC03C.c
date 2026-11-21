#include "obj_BC03C.h"

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

/* ROM 0x0BC03C: init */
int Obj_BC03C_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x10u, 0x000548E8u);
    wr16(m, obj + 0x14u, 0x0010u);
    wr16(m, obj + 0x18u, 0x0060u);
    wr16(m, obj + 0x60u, 0x007Fu);
    wr32(m, obj + 0x00u, 0x000BC064u);
    return 0;
}
