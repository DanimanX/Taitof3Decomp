#include "obj_E800C.h"
#include "obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E800C: 15-segment column riding on the parent */
int Obj_E800C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x10u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    uint16_t d0 = rd8(m, obj + 0x22u) != 0u ? 0xFFF3u : 0x011Du;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));
    uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
    wr8(m, obj + 0x23u, t);
    if (t == 0u) {
        uint8_t s = (uint8_t)(rd8(m, obj + 0x06u) - 1u);
        wr8(m, obj + 0x06u, s);
        if ((int8_t)s < 0) {
            Obj_StartDying(m, obj);
            return 0;
        }
        Obj_SetFrame(m, obj, rd32(m, 0x000E8140u + (uint32_t)s * 4u));
        wr8(m, obj + 0x23u, 0x04u);
    }
    /* 0x0E806A */
    Coll_AddHazard(m, obj);
    Obj_DrawSprite(m, obj);
    for (int i = 0; i < 14; i++) {
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x10u));
        Obj_DrawSprite(m, obj);
    }
    return 0;
}
