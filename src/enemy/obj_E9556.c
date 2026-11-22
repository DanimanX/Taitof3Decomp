#include "obj_E9556.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E9556 */
int Obj_E9556_Tick(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, rd32(m, obj + 0x08u) + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_AnimStep(m, obj);
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));
    Obj_DrawSprite(m, obj);
    return 0;
}
