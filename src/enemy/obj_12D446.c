#include "obj_12D446.h"
/* ROM 0x12D446 */
int Obj_12D446_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D446 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D44A bgt */
        /* 0x12D43E */
        wr32(m, obj + 0x00u, 0x0012D446u);
        return 0;
    }
    /* 0x12D44C */
    wr16(m, F3_A5 + 0x32A6u, 0x0000u);
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u)); /* bfset {3:1} */
    wr16(m, obj + 0x06u, 0x0028u);
    /* 0x12D45C beq: Z was set by the clr.w above, so this never branches */
    wr32(m, obj + 0x00u, 0x0012D466u);
    return 0;
}
