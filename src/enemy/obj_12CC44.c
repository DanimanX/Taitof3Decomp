#include "obj_12CC44.h"
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

/* ROM 0x12CC44 */
int Obj_12CC44_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj + 0x00u, 0x0012CC44u);   /* 0x12CC3C */
        return 0;
    }
    sub_001660(m, 0x001326AEu, 0x5E80u, 0x20u);
    uint32_t d0 = Obj_Spawn(m, obj, 0x0012D0E6u, 3u, 2u, 0u);

    /* 0x12CF44 */
    wr32(m, F3_A5 + 0x30DEu, d0);
    wr16(m, F3_A5 + 0x32A0u, 0u);
    wr8(m, F3_A5 + 0x32AEu, 0u);
    wr8(m, F3_A5 + 0x329Cu, 0u);
    wr8(m, F3_A5 + 0x329Du, 0u);
    wr16(m, F3_A5 + 0x329Eu, 0u);
    Obj_Spawn(m, obj, 0x0012EA24u, 3u, 2u, 0u);
    Obj_Spawn(m, obj, 0x0012F26Au, 3u, 2u, 0u);
    Obj_Spawn(m, obj, 0x0011C1DEu, 4u, 1u, 0u);
    Obj_Spawn(m, obj, 0x0011C39Au, 4u, 1u, 0u);
    wr32(m, obj + 0x00u, 0x0012CFD4u);       /* 0x12CFCC */
    return 0;
}
