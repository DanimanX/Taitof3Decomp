#include "obj_BCF30.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0BCF30: draw a row of 7 copies (x = i*0x30) scaled by +0x60 */
int Obj_BCF30_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x5Cu, 0x0006u);
    int16_t i;
    do {
        uint16_t d0w = (uint16_t)(0x000Bu - rd16(m, obj + 0x60u));
        d0w = (uint16_t)(d0w << 4);
        wr8(m, obj + 0x1Eu, (uint8_t)d0w);
        wr32(m, obj + 0x18u, rd32(m, obj + 0x44u));
        uint32_t d0 = ((uint32_t)rd8(m, obj + 0x1Eu) * 0x30u) << 8;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x5Cu) * 0x30u)); /* .w: high word only */
        Obj_Animate(m, obj, rd32(m, obj + 0x48u));
        Obj_DrawSprite(m, obj);
        i = (int16_t)(rd16(m, obj + 0x5Cu) - 1u);
        wr16(m, obj + 0x5Cu, (uint16_t)i);
    } while (i >= 0);

    int16_t n = (int16_t)(rd16(m, obj + 0x60u) - 1u);
    wr16(m, obj + 0x60u, (uint16_t)n);
    if (n >= 0) {
        wr32(m, obj + 0x00u, 0x000BCF30u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
