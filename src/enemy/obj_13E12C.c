#include "obj_13E12C.h"

/* ROM 0x001660: queue a palette upload {count.w, src.l, dst.w} (max 0x20) */
static void sub_001660(const f3mem_t *m, uint32_t src, uint16_t dst, uint16_t count)
{
    uint16_t n = rd16(m, F3_A5 - 0x5DF0u);
    if ((int16_t)n >= 0x20)
        return;
    uint32_t a2 = F3_A5 - 0x6750u + (uint32_t)(int32_t)(int16_t)(uint16_t)(n << 3);
    wr16(m, a2, count);
    wr32(m, a2 + 2u, src);
    wr16(m, a2 + 6u, dst);
    wr16(m, F3_A5 - 0x5DEEu, 0x0002u);
    wr16(m, F3_A5 - 0x5DF0u, (uint16_t)(n + 1u));
}

/* ROM 0x13E12C */
int Obj_13E12C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x13E12C: queue 0x20 colours from 0x16980 to 0xA00 */
    sub_001660(m, 0x00016980u, 0x0A00u, 0x0020u);
    wr32(m, obj + 0x00u, 0x0013E150u);
    return 0;
}
