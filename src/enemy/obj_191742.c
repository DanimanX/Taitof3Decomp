#include "obj_191742.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x191742: continue the slide; frees itself once hi(+0x60)+byte(+0x1E) > 0x100 */
int Obj_191742_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 5u));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 1u));
    uint16_t d0 = (uint16_t)(rd8(m, obj + 0x60u) + rd8(m, obj + 0x1Eu));
    if ((int16_t)d0 > 0x100) {
        wr16(m, F3_A5 + 0x4B92u, (uint16_t)(rd16(m, F3_A5 + 0x4B92u) - 1u));
        Obj_Free(m, obj);
        return 0;
    }
    wr8(m, obj + 0x1Eu, (uint8_t)d0);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + 0x40u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (rd8(m, obj + 0x1Eu) >> 1)));
    Obj_DrawSprite(m, obj);
    wr32(m, obj, 0x191742u);
    return 0;
}
