#include "obj_17F74E.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17F74E: boss wing/segment draw with +0x1E progress, free at 0xF0 */
int Obj_17F74E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t a1 = rd32(m, obj + 0x10u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(rd16(m, a1 + 2u) << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(rd16(m, a1) << 3)));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 4u));
    uint16_t d1 = (uint16_t)(rd16(m, a1) << 4);
    uint16_t d0 = (uint16_t)rd8(m, obj + 0x1Eu);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(((uint32_t)d1 * d0) >> 9)));
    Obj_DrawSprite(m, obj);
    if (rd8(m, obj + 0x1Eu) == 0xF0u)
        Obj_Free(m, obj);
    else
        wr32(m, obj + 0x00u, 0x0017F74Eu);
    return 0;
}
