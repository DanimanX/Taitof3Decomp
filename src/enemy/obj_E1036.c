#include "obj_E1036.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E1036: follow the parent (+0x20 below it) and draw both */
int Obj_E1036_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0020u));
    Obj_DrawSprite(m, obj);
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));
    return 0;
}
