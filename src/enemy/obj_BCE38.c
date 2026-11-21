#include "obj_BCE38.h"
/* ROM 0xBCE38 */
int Obj_BCE38_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BCE38 */
    /* 0x0CBD86: clear the frame state */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x0Cu));
    wr16(m, obj + 0x60u, 0x000Bu);
    /* 0x0BCE4A */
    wr32(m, obj, 0x000BCE52u);
    return 0;
}
