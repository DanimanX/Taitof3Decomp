#include "obj_17AA36.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x17AA36: parent-attached effect + random 179ED0 sparkle spawner */
int Obj_17AA36_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {
        wr8(m, obj + 0x23u, (uint8_t)(rd8(m, obj + 0x23u) | 0x04u));
        wr16(m, obj + 0x06u, 1u);
        wr32(m, obj + 0x00u, 0x0017AADAu);
        return 0;
    }
    uint32_t parent = rd32(m, obj + 0x08u);
    uint16_t x = (uint16_t)(rd16(m, parent + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t y = (uint16_t)(rd16(m, parent + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, x);
    wr16(m, obj + 0x54u, y);
    wr16(m, obj + 0x14u, x);
    wr16(m, obj + 0x18u, y);
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t spr = rd32(m, obj + 0x10u);
    int16_t dy = (int16_t)((int16_t)rd16(m, spr) << 3);
    int16_t dx = (int16_t)((int16_t)rd16(m, spr + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dx));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));
    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    Obj_DrawSprite(m, obj);
    if (Obj_RandRange(m, 0x14u) == 0u)
        (void)Obj_Spawn(m, obj, 0x00179ED0u, 5u, 2u, 0u);
    wr32(m, obj + 0x00u, 0x0017AA36u);
    return 0;
}

/* ROM 0x17AA2E: entry reached by fall-through and by jmp 0x17AA2E at
 * 0x18BB34 -- just arms 0x17AA36 for the next tick. */
int Obj_17AA2E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0017AA36u);
    return 0;
}
