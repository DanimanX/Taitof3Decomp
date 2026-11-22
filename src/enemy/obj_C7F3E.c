#include "obj_C7F3E.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0C7F3E: overlay drawn on each live player; steps through the
 * {time.w, anim.l} records at +0x0C */
int Obj_C7F3E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_Animate(m, obj, rd32(m, obj + 0x36u));
    uint32_t a1 = 0x0041B190u;
    for (int i = 0; i < 2; i++, a1 += 0x8Au) {
        if ((int16_t)rd16(m, a1 + 0x4Cu) < 0 || (rd8(m, a1 + 0x65u) & 0x04u) != 0u)
            continue;
        wr16(m, obj + 0x14u, rd16(m, a1 + 0x14u));
        wr16(m, obj + 0x18u, rd16(m, a1 + 0x18u));
        if ((int16_t)rd16(m, a1 + 0x4Eu) > 0) {
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x20u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x25u));
        } else {
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x23u));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x2Bu));
        }
        Obj_DrawSprite(m, obj);
    }
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t <= 0) {
        /* 0x0C7F1A */
        uint32_t r = rd32(m, obj + 0x0Cu);
        uint16_t n = rd16(m, r); r += 2u;
        wr16(m, obj + 0x06u, n);
        if ((int16_t)n < 0) {
            Obj_Free(m, obj);
            return 0;
        }
        wr32(m, obj + 0x36u, rd32(m, r)); r += 4u;
        wr32(m, obj + 0x0Cu, r);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
    }
    wr32(m, obj + 0x00u, 0x000C7F3Eu);
    return 0;
}
