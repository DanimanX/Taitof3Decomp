#include "obj_E76FC.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0E76FC: rides the parent (+0x08) with a y offset of 0x2F, or -0x1F
 * when +0x22 is set; dies with the parent (+0x06 = 0) or when its script
 * timer +0x4C runs out, otherwise steps the script and draws. */
int Obj_E76FC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    if (rd8(m, a1 + 0x06u) == 0u) {
        Obj_StartDying(m, obj);                     /* 0x0CEE74 */
        return 0;
    }
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    /* moveq #0x2F / moveq #-0x1F (sign-extended) ; add.w D0,(0x18,A0) */
    uint16_t d0 = (rd8(m, obj + 0x22u) != 0u) ? 0xFFE1u : 0x002Fu;
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0));

    Obj_StepScript(m, obj);                         /* 0x0D0690 */
    if (rd16(m, obj + 0x4Cu) == 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    return 0;
}
