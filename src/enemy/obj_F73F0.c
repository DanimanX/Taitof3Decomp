#include "obj_F73F0.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0F73F0: the 0x0F7398 part's tick. Steps its script; dies when the
 * script ends, when the parent is gone (+0x06 = 0) or in phase >= 4;
 * otherwise follows the parent at its (+0x50, +0x54) offset and draws. */
int Obj_F73F0_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_StepScript(m, obj);                         /* 0x0D0690 */
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    uint32_t p = rd32(m, obj + 0x08u);
    uint8_t ph = rd8(m, p + 0x06u);
    if (ph == 0u || ph >= 4u) {                     /* beq / bcc (unsigned) */
        Obj_StartDying(m, obj);
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x54u)));
    Obj_DrawSprite(m, obj);
    return 0;
}
