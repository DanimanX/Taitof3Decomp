#include "obj_12D466.h"
/* ROM 0x12D466 */
int Obj_12D466_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D466 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D46A bgt */
        /* 0x12D45E */
        wr32(m, obj + 0x00u, 0x0012D466u);
        return 0;
    }
    /* 0x12D46C */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u)); /* bfset {3:1} */
    wr16(m, obj + 0x06u, 0x0028u);
    /* 0x12D478 beq: Z was set by the bfset (byte 0x00), never taken */
    wr32(m, obj + 0x00u, 0x0012D482u);
    return 0;
}
