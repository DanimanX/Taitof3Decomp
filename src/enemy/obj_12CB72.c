#include "obj_12CB72.h"
#include "../objects/obj_pool.h"

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

/* ROM 0x12CB72 (the beq at 0x12CC3A is never taken) */
int Obj_12CB72_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, F3_A5 + 0x30EEu, 0u);
    wr8(m, F3_A5 + 0x30F2u, 0u);
    wr8(m, F3_A5 + 0x30F4u, 0u);
    wr8(m, obj + 0x59u, 0u);
    Obj_Spawn(m, obj, 0x00130044u, 3u, 2u, 0x00130256u);
    Obj_Spawn(m, obj, 0x00131408u, 4u, 1u, 0u);
    sub_001660(m, 0x00018300u, 0x5100u, 0x1Eu);
    sub_001660(m, 0x00018280u, 0x5080u, 0x1Bu);
    sub_001660(m, 0x00018200u, 0x5000u, 0x1Bu);
    sub_001660(m, 0x00019080u, 0x5F80u, 0x1Eu);
    wr32(m, F3_A5 + 0x30DAu, 0x00960000u);
    wr16(m, obj + 0x06u, 0x001Eu);
    wr32(m, obj + 0x00u, 0x0012CC44u);       /* 0x12CC3C */
    return 0;
}
