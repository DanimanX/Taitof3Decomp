#include "obj_10BD20.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x10BD20: init attached part, draw sprite, arm 0x10BD88 */
int Obj_10BD20_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x00108734u);
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x10u));
    Obj_StartBehaviour(m, obj, 0x0010BDC0u);
    wr16(m, obj + 0x50u, (uint16_t)-0x0016);
    wr16(m, obj + 0x54u, (uint16_t)-0x0004);
    uint32_t a2 = rd32(m, obj + 0x08u);
    if ((int16_t)rd16(m, a2 + 0x24u) <= 1)
        Obj_StartBehaviour(m, obj, 0x0010BDF0u);
    wr32(m, obj + 0x14u, rd32(m, a2 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr32(m, obj + 0x18u, rd32(m, a2 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x54u)));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0010BD88u);
    return 0;
}
