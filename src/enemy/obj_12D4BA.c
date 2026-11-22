#include "obj_12D4BA.h"
/* ROM 0x12D4BA */
int Obj_12D4BA_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);      /* 0x12D4BA subq.w */
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {                                           /* 0x12D4BE bgt */
        /* 0x12D4B2 */
        wr32(m, obj + 0x00u, 0x0012D4BAu);
        return 0;
    }
    /* 0x12D4C0 -> 0x12D218 */
    wr16(m, F3_A5 + 0x32A6u, 0x0006u);
    wr16(m, F3_A5 + 0x32A8u, 0x0002u);
    /* 0x12D224 */
    wr32(m, obj + 0x00u, 0x0012D22Cu);
    return 0;
}
