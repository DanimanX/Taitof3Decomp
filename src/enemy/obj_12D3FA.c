#include "obj_12D3FA.h"
/* ROM 0x12D3FA */
int Obj_12D3FA_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D3FA subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D3FE bgt */
        /* 0x12D3F2 */
        wr32(m, obj + 0x00u, 0x0012D3FAu);
        return 0;
    }
    /* 0x12D400 */
    wr16(m, obj + 0x06u, 0x003Cu);
    wr16(m, F3_A5 + 0x32A8u, 0x0002u);
    /* 0x12D40C */
    wr32(m, obj + 0x00u, 0x0012D414u);
    return 0;
}
