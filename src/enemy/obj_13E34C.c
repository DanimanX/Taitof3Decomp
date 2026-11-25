#include "obj_13E34C.h"
#include "../objects/obj_pool.h"

/* ROM 0x13E34C: zoom-wobble tick. While the A5+0x32FE mode byte is < 2 it
 * accumulates +0x18 into A5-0x5D94 (mirrored at A5+0x4218), shifts the 0xE8
 * line-ram words at 0x62A400 by +/-(+0x14 >> 10) and steps +0x14/+0x18;
 * otherwise the object frees itself. */
int Obj_13E34C_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int8_t)rd8(m, F3_A5 + 0x32FEu) >= 2) {
        Obj_Free(m, obj);                                        /* ROM 0x13E3B6 */
        return 0;
    }
    uint32_t acc = rd32(m, F3_A5 - 0x5D94u) + rd32(m, obj + 0x18u);
    wr32(m, F3_A5 - 0x5D94u, acc);
    wr32(m, F3_A5 + 0x4218u, acc);

    uint32_t a1 = 0x0062A400u;
    uint32_t a2 = a1 + 0xE8u;
    int32_t d0 = (int32_t)rd32(m, obj + 0x14u);
    if (rd8(m, F3_A5 - 0x5D58u) != 0u) d0 = (int32_t)(0u - (uint32_t)d0);
    d0 >>= 10;                                                   /* asr.l #8; asr.l #2 */
    uint16_t d0w = (uint16_t)d0;
    for (int i = 0; i <= 0x73; i++) {
        wr16(m, a1, (uint16_t)((((rd16(m, a1) ^ 0x3Fu) + d0w) & 0xFFFFu) ^ 0x3Fu));
        a1 += 2u;
        wr16(m, a2, (uint16_t)((((rd16(m, a2) ^ 0x3Fu) - d0w) & 0xFFFFu) ^ 0x3Fu));
        a2 += 2u;
    }
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + 0x600u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - 0x700u);
    wr32(m, obj + 0x00u, 0x0013E34Cu);                           /* ROM 0x13E344 */
    return 0;
}
