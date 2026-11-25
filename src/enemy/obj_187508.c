#include "obj_187508.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x187508 */
int Obj_187508_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x5Cu, (uint16_t)(rd16(m, obj + 0x5Cu) + 0x0040u));
    int16_t d0s = (int16_t)rd16(m, obj + 0x42u) >> 6;
    int16_t d1s = (int16_t)rd16(m, obj + 0x5Cu) >> 6;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)((int32_t)d0s << 14));
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)((int32_t)d1s << 14));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t oy = (int16_t)(rd16(m, a1) << 3);
    int16_t ox = (int16_t)(rd16(m, a1 + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - ox));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - oy));
    if ((int16_t)rd16(m, obj + 0x50u) <= -0x64) {
        Obj_Free(m, obj);
        return 0;
    }
    if ((int16_t)rd16(m, obj + 0x54u) < (int16_t)rd16(m, obj + 0x64u)) {
        Obj_BossMapScreenPos(m, obj);
        Obj_DrawSprite(m, obj);
        wr32(m, obj, 0x00187508u);
        return 0;
    }
    if (rd8(m, F3_A5 + 0x1D58u) & 0x02u) {
        uint16_t idx = (uint16_t)((rd16(m, F3_A5 - 0x1456u) >> 1) & 0x03u) * 4u;
        wr32(m, obj + 0x10u, rd32(m, 0x0018760Au + idx));
        Obj_ExplodeTick(m, obj);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
