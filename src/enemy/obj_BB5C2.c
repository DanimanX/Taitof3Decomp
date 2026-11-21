#include "obj_BB5C2.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0BB5C2: fly left with growing speed, shrinking, until off screen */
int Obj_BB5C2_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - rd32(m, obj + 0x4Cu));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) - 0x6000u);
    Obj_Animate(m, obj, rd32(m, obj + 0x60u));
    if (rd8(m, obj + 0x1Fu) != 0u) {
        wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 1u));
        wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 1u));
    }

    /* 0x0BB5EC */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    uint32_t d0 = ((uint32_t)rd8(m, obj + 0x1Fu) * 0x50u) << 8;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));
    d0 = ((uint32_t)rd8(m, obj + 0x1Eu) * 0x40u) << 8;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x4Cu, rd32(m, obj + 0x4Cu) + 0x1C0u);
    if ((int16_t)rd16(m, obj + 0x14u) < -0xA0) {
        Obj_Free(m, obj);
        return 0;
    }
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + 1u));
    wr32(m, obj + 0x00u, 0x000BB5C2u);
    return 0;
}
