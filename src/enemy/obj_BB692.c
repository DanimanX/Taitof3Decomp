#include "obj_BB692.h"
#include "../video/sprite.h"

/* ROM 0x0BB692: circle around (+0x44, +0x48) using the sine table 0x27E40 */
int Obj_BB692_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));

    uint16_t i = (uint16_t)(rd16(m, obj + 0x60u) & 0xFFu);
    int32_t d0 = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)i * 2u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + ((uint32_t)d0 << 5));
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 - 0x5DA0u));

    i = (uint16_t)((rd16(m, obj + 0x60u) + 0x40u) & 0xFFu);
    d0 = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)i * 2u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + ((uint32_t)d0 << 6));

    Obj_DrawSprite(m, obj);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + 1u));
    wr32(m, obj + 0x00u, 0x000BB692u);
    return 0;
}
