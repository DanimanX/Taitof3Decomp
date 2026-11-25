#include "obj_13DEAE.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x13DEAE */
int Obj_13DEAE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, rd16(m, F3_A5 + 0x32FAu));
    wr16(m, obj + 0x18u, rd16(m, F3_A5 + 0x32FCu));
    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t d0 = (int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x100u);
    d0 = (int16_t)((int32_t)d0 * (int16_t)rd16(m, a1));
    d0 >>= 5;
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) + d0));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) + d0));
    Obj_DrawSprite(m, obj);
    uint16_t prev = rd16(m, obj + 0x1Eu);
    wr16(m, obj + 0x1Eu, (uint16_t)(prev - 0x2020u));
    if (prev >= 0x2020u) {
        wr32(m, obj, 0x0013DEAEu);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
