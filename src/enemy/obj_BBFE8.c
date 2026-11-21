#include "obj_BBFE8.h"
/* ROM 0xBBFE8 */
int Obj_BBFE8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BBFE8 */
    /* 0x0CBD86: clear the frame state */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr16(m, obj + 0x14u, 0x0070u);
    wr16(m, obj + 0x18u, 0x0010u);
    wr16(m, obj + 0x60u, 0x007Fu);
    wr32(m, obj, 0x000BC008u);
    return 0;
}
