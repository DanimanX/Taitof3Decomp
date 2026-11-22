#include "obj_12D382.h"
/* ROM 0x12D382 */
int Obj_12D382_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D382 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D386 bgt */
        /* 0x12D37A */
        wr32(m, obj + 0x00u, 0x0012D382u);
        return 0;
    }
    /* 0x12D388 */
    wr16(m, obj + 0x06u, 0x000Fu);
    wr16(m, F3_A5 + 0x32A8u, 0x0001u);
    /* 0x12D394 */
    wr32(m, obj + 0x00u, 0x0012D39Cu);
    return 0;
}
