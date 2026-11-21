#include "obj_E1886.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0D4A12: D0w = high word of (sin_table[D1w] * D0), via the
 * shift/add multiply jump table 0x0D4E5A (exact multiply for k = 0..50) */
static uint16_t sub_0D4A12(const f3mem_t *m, uint16_t k, uint16_t idx)
{
    uint32_t v = rd32(m, 0x000D4A5Au + (uint32_t)((int32_t)(int16_t)idx * 4));
    v *= (uint32_t)k;
    return (uint16_t)(v >> 16);   /* swap D0 */
}

/* ROM 0x0E1886: child attached to the parent at +0x08 */
int Obj_E1886_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr16(m, obj + 0x1Eu, rd16(m, a1 + 0x1Eu));
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));

    /* 0x0E18A6 */
    uint16_t d0w = sub_0D4A12(m, 0x20u, rd8(m, obj + 0x1Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0w));
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    return 0;
}
