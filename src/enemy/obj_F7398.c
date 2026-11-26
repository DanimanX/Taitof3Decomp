#include "obj_F7398.h"
#include "obj_E8B72.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0F7398: init of a parent-attached part. Stats 0x0F8214; the part
 * index n = +0x10 (stored negative) selects the 8-byte record
 * 0x0F743E[-n-1] = {dx.w, dy.w, behaviour.l}; it sits at the parent's
 * position + (dx, dy), draws and arms 0x0F73F0. */
int Obj_F7398_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_0CEDE4(m, obj, 0x000F8214u);               /* lea (0xE7A,PC) */

    /* neg.w ; subq.w #1 ; lea (0x92,PC,D0.w*8),A1 -- full-format ext at
     * 0x0F73AC, so the base is 0x0F73AC + 0x92 = 0x0F743E */
    uint16_t d0 = (uint16_t)(0u - rd16(m, obj + 0x10u) - 1u);
    uint32_t a1 = 0x000F743Eu + (uint32_t)(int32_t)(int16_t)d0 * 8u;
    wr16(m, obj + 0x50u, rd16(m, a1));
    wr16(m, obj + 0x54u, rd16(m, a1 + 2u));
    Obj_SetAnim(m, obj, rd32(m, a1 + 4u));          /* 0x0D0678 */

    uint32_t p = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x54u)));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x000F73F0u);
    return 0;
}
