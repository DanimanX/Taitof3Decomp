#include "obj_1199A8.h"

/* ROM 0x1199A8 */
int Obj_1199A8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x001199A8u);   /* 0x1199A0 */
        return 0;
    }
    wr32(m, obj + 0x14u, 0x01400000u);
    wr32(m, obj + 0x18u, 0xFFE80000u);
    wr16(m, obj + 0x06u, 0x016Cu);
    wr32(m, obj + 0x00u, 0x001199CCu);       /* 0x1199C4 */
    return 0;
}
