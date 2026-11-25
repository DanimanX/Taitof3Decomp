#include "obj_1873D6.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x1873D6 */
int Obj_1873D6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 0xFF80u));
    int16_t d0s = (int16_t)rd16(m, obj + 0x42u) >> 6;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)d0s << 14));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t oy = (int16_t)(rd16(m, a1) << 3);
    int16_t ox = (int16_t)(rd16(m, a1 + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - ox));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - oy));
    Obj_BossMapScreenPos(m, obj);
    Obj_DrawSprite(m, obj);
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        wr32(m, obj, 0x001873D6u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
