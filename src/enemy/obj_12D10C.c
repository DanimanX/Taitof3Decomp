#include "obj_12D10C.h"

/* ROM 0x12D10C */
int Obj_12D10C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0012D10Cu);   /* 0x12D104 */
        return 0;
    }
    /* 0x12D112 */
    wr16(m, F3_A5 + 0x32A6u, 0x003Cu);
    uint32_t a1 = rd32(m, rd32(m, F3_A5 + 0x32AAu) + 0x48u);
    wr32(m, a1 + 0x56u, 0x01000000u);
    wr32(m, a1 + 0x5Au, 0x00030000u);
    wr32(m, a1 + 0x5Eu, 0x00300000u);
    wr32(m, obj + 0x00u, 0x0012D140u);       /* 0x12D138 */
    return 0;
}
