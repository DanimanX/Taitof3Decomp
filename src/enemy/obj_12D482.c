#include "obj_12D482.h"
/* ROM 0x12D482 */
int Obj_12D482_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D482 subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D486 bgt */
        /* 0x12D47A */
        wr32(m, obj + 0x00u, 0x0012D482u);
        return 0;
    }
    /* 0x12D488 */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x10u)); /* bfset {3:1} */
    wr16(m, obj + 0x06u, 0x0064u);
    /* 0x12D494 beq: Z was set by the bfset (byte 0x00), never taken */
    wr32(m, obj + 0x00u, 0x0012D49Eu);
    return 0;
}
