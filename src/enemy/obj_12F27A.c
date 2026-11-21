#include "obj_12F27A.h"

/* ROM 0x12F27A: wait +0x06 frames, then init */
int Obj_12F27A_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t n = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)n);
    if (n > 0) {
        wr32(m, obj + 0x00u, 0x0012F27Au);
        return 0;
    }
    wr32(m, obj + 0x14u, 0u);
    wr32(m, obj + 0x18u, 0u);
    wr16(m, obj + 0x42u, 0u);
    wr16(m, obj + 0x20u, 0x00C0u);
    wr8(m, obj + 0x22u, 0x00u);   /* sf */
    wr32(m, obj + 0x00u, 0x0012F29Eu);
    return 0;
}
