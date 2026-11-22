#include "obj_12D414.h"
/* ROM 0x12D414 */
int Obj_12D414_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)(rd16(m, F3_A5 + 0x32A2u) - 0x0088u)); /* 0x12D414 */
    wr16(m, F3_A5 + 0x32A4u, (uint16_t)(rd16(m, F3_A5 + 0x32A4u) - 0x0059u)); /* 0x12D41A */

    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D420 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D424 bgt */
        /* 0x12D40C */
        wr32(m, obj + 0x00u, 0x0012D414u);
        return 0;
    }
    /* 0x12D426 */
    wr16(m, F3_A5 + 0x32A2u, 0x8000u);
    wr16(m, F3_A5 + 0x32A4u, 0x0000u);
    wr16(m, F3_A5 + 0x32A6u, 0x0002u);
    wr16(m, obj + 0x06u, 0x003Cu);
    /* 0x12D43C beq: Z was set by the move.w above, never taken */
    wr32(m, obj + 0x00u, 0x0012D446u);
    return 0;
}
