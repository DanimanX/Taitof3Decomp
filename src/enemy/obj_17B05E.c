#include "obj_17B05E.h"
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

/* ROM 0x17B05E: palette cycler (16 frames) until A5+0x4B38 == -1 */
int Obj_17B05E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)(rd16(m, F3_A5 + 0x878u) & 0x0Fu);
    sub_001660(m, 0x17B09Eu + (uint32_t)d0 * 4u, 0x6C00u, 0x20u);
    if (rd16(m, F3_A5 + 0x4B38u) != 0xFFFFu) {
        wr32(m, obj, 0x17B05Eu);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
