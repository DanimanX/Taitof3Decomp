#include "obj_BCFF4.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBCFF4 */
int Obj_BCFF4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BCFF4: sprite 1 */
    wr32(m, obj + 0x10u, rd32(m, obj + 0x50u));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u) - 0x310000u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u) - 0x540000u);
    wr16(m, obj + 0x1Eu, 0x2020u);
    uint16_t d0w = (uint16_t)(rd16(m, obj + 0x60u) << 1);
    d0w &= 0x00FFu;
    /* lea (0x27E40).l: sin table, .w entries */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) +
         (uint32_t)((int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)d0w * 2u) * 0x0A));
    Obj_DrawSprite(m, obj);

    /* 0x0BD03E: sprite 2 */
    wr32(m, obj + 0x10u, rd32(m, obj + 0x4Cu));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u) - 0x300000u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u) - 0x480000u);
    wr16(m, obj + 0x1Eu, 0u);
    d0w = (uint16_t)(rd16(m, obj + 0x60u) << 1);
    d0w &= 0x00FFu;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) +
         (uint32_t)((int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)d0w * 2u) * 0x06));
    Obj_DrawSprite(m, obj);

    /* 0x0BD086 */
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) - 0x2000u);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BCFEC */
        wr32(m, obj, 0x000BCFF4u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
