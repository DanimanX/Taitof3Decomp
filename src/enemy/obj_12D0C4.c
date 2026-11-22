#include "obj_12D0C4.h"
#include "../objects/obj_pool.h"

/* ROM 0x12D0CA -> jmp 0x11AE2C: clear a line-RAM row, flag, free self */
static void finish(const f3mem_t *m, uint32_t obj)
{
    wr8(m, F3_A5 + 0x30F2u, (uint8_t)(rd8(m, F3_A5 + 0x30F2u) | 0x02u));
    wr8(m, obj + 0x59u, 0xFFu);
    wr16(m, F3_A5 + 0x32FAu, 0x00A0u);
    wr16(m, F3_A5 + 0x32FCu, 0x0074u);
    /* 0x11AE2C */
    wr8(m, F3_A5 + 0x30F4u, 0u);
    uint32_t a0 = 0x00627600u + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (int i = 0; i <= 0xE7; i++) {
        wr16(m, a0, 0xDDDDu);
        a0 += 2u;
    }
    wr8(m, F3_A5 - 0x144Cu, (uint8_t)(rd8(m, F3_A5 - 0x144Cu) | 0x08u));
    Obj_Free(m, obj);
}

/* ROM 0x12D0C4 */
int Obj_12D0C4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0012D0C4u);   /* 0x12D0BC */
        return 0;
    }
    finish(m, obj);
    return 0;
}
