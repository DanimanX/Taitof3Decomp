#include "obj_12D0E6.h"

/* ROM 0x12D0E6 (the beq at 0x12D102 is never taken) */
int Obj_12D0E6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 + 0x30F3u, 0xFFu);
    wr16(m, F3_A5 + 0x32A2u, 0x8000u);
    wr16(m, F3_A5 + 0x32A4u, 0u);
    wr16(m, F3_A5 + 0x32A6u, 0u);
    wr16(m, F3_A5 + 0x32A8u, 0u);
    wr16(m, obj + 0x06u, 0x0002u);
    wr32(m, obj + 0x00u, 0x0012D10Cu);       /* 0x12D104 */
    return 0;
}
