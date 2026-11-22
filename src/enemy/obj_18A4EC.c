#include "obj_18A4EC.h"
#include "../video/sprite.h"

/* ROM 0x18A4EC: cross-hair object. Publish the negated parent position
 * (A4 = (A0+8)) as the screen cursor (A5-0x5DA0 / A5-0x5D9C), copy the
 * parent position offset by +0x52/+0x56 into +0x14/+0x18, centre by the
 * frame size, draw, and loop to 0x18A4E4 (see work/disasm/bundle_18A4EC.s). */
int Obj_18A4EC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);

    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    d0 = (uint16_t)(0u - d0);                 /* neg.w */
    d1 = (uint16_t)(0u - d1);
    wr16(m, F3_A5 - 0x5DA0, d0);
    wr16(m, F3_A5 - 0x5D9Cu, d1);

    d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x52u));
    d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x56u));
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    /* 0x18A524: movea.l (0x10,A0),A1; move.w (A1)+,D1; move.w (A1)+,D0 */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t h = (uint16_t)(rd16(m, a1) << 3);
    a1 += 2u;
    uint16_t w = (uint16_t)(rd16(m, a1) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - h));

    Obj_DrawSprite(m, obj);                   /* jsr 0x001282 */

    /* 0x18A4E4: move.l #0x18A4EC,(A0); rts */
    wr32(m, obj + 0x00u, 0x0018A4ECu);
    return 0;
}
