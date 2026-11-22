#include "obj_12D49E.h"
/* ROM 0x12D49E */
int Obj_12D49E_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D49E subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D4A2 bgt */
        /* 0x12D496 */
        wr32(m, obj + 0x00u, 0x0012D49Eu);
        return 0;
    }
    /* 0x12D4A4 */
    wr8(m, F3_A5 + 0x30EEu, (uint8_t)(rd8(m, F3_A5 + 0x30EEu) | 0x20u)); /* bfset {2:1} */
    wr16(m, obj + 0x06u, 0x00A0u);
    /* 0x12D4B0 beq: Z was set by the bfset (byte 0x00), never taken */
    wr32(m, obj + 0x00u, 0x0012D4BAu);
    return 0;
}
