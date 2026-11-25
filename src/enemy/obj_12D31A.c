#include "obj_12D31A.h"

/* ROM 0x12D31A */
int Obj_12D31A_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x32A2u) < -0x4000)
        wr16(m, F3_A5 + 0x32A6u, 6u);
    int16_t v = (int16_t)(rd16(m, F3_A5 + 0x32A2u) - 0x0280u);
    wr16(m, F3_A5 + 0x32A2u, (uint16_t)v);
    if (v <= 0) {
        wr32(m, obj, 0x0012D31Au);
        return 0;
    }
    wr16(m, F3_A5 + 0x32A6u, 0u);
    wr16(m, F3_A5 + 0x32A2u, 0x8000u);
    wr16(m, obj + 0x06u, 0x001Eu);
    wr32(m, obj, 0x0012D350u);
    return 0;
}
