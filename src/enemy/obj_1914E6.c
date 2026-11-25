#include "obj_1914E6.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x1914E6: animated piece moved by a 10-bit signed velocity from +0x42 */
int Obj_1914E6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t w = (uint16_t)(rd16(m, obj + 0x42u) - 0x40u);
    wr16(m, obj + 0x42u, w);
    int32_t d0 = (int32_t)((int16_t)w >> 6);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)(d0 * 16384));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1 = rd16(m, a1);
    uint16_t d0w = rd16(m, a1 + 2u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(d0w << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(d1 << 3)));
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        wr32(m, obj, 0x1914E6u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
