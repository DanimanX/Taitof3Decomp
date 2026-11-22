#include "obj_11946C.h"

/* ROM 0x11946C */
int Obj_11946C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0011946Cu);   /* 0x119464 */
        return 0;
    }
    wr32(m, obj + 0x10u, 0x00119569u);
    wr16(m, obj + 0x14u, 0x000Cu);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 2u));
    wr16(m, obj + 0x1Cu, 0x0007u);
    wr16(m, obj + 0x06u, 0x0003u);           /* 0x11948A: the beq is never taken */
    wr32(m, obj + 0x00u, 0x0011949Au);
    return 0;
}
