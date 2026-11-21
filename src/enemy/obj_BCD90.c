#include "obj_BCD90.h"
/* ROM 0xBCD90 */
int Obj_BCD90_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BCD90 */
    /* 0x0CBD86: clear the frame state */
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
    /* 0x0BCD96 */
    wr32(m, obj + 0x5Cu, 0x000BCDF4u);
    wr16(m, obj + 0x14u, rd16(m, obj + 0x0Cu));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x0Eu));
    wr32(m, obj + 0x44u, 0x00010000u);
    wr16(m, obj + 0x60u, 0x000Fu);
    /* 0x0BCDB8 */
    wr32(m, obj, 0x000BCDC0u);
    return 0;
}
