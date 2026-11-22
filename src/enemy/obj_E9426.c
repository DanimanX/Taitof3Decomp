#include "obj_E9426.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E9426: part following the parent; always redraws the parent */
int Obj_E9426_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint8_t st = rd8(m, a1 + 0x06u);
    if (st == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    if (st != 1u && st != 3u) {
        wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
        wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x20u));
        Obj_AnimStep(m, obj);
        Obj_DrawSprite(m, obj);
    }
    Obj_DrawSprite(m, rd32(m, obj + 0x08u));   /* 0x0E9462: A0 = parent */
    return 0;
}
