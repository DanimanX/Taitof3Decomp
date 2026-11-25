#include "obj_13151E.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x12FF36: colour override from the boss/current-enemy slot A5+0x32AA */
static void sub_12FF36(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x32AAu);
    uint8_t d0 = rd8(m, a1 + 0x1Du);
    if (d0 == rd8(m, a1 + 0x4Fu))
        d0 = rd8(m, obj + 0x4Fu);
    wr8(m, obj + 0x1Du, d0);
}

/* ROM 0x13151E: the object pops out of its parent (+8) along the signed
 * offset in +0x16/+0x1A while the zoom word +0x1E (0x100 = 1:1) winds down.
 * Each tick the offset closes by (zoom-0x100)>>5 and the zoom drops by
 * 0x1010; once the zoom would wrap, the offsets/state are cleared and the
 * object hands over to 0x13157A. */
int Obj_13151E_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x13151E btst.b #0,(A5+0x30F2) ; bne.w 0x131664 */
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u) {
        /* 0x131664: move.l A0,-(SP) ; jsr 0x105EC */
        Obj_Free(m, obj);
        return 0;
    }

    {
        uint32_t parent = rd32(m, obj + 0x08u);
        uint16_t d0w = (uint16_t)(rd16(m, obj + 0x16u) + rd16(m, parent + 0x14u));
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Au) + rd16(m, parent + 0x18u));
        /* 0x13153C: D2w = ((uint8)(A0+0x1E) - 0x100) >> 5 (asr) */
        int16_t d2w = (int16_t)(((int16_t)(uint16_t)(uint8_t)rd8(m, obj + 0x1Eu) - 0x100) >> 5);
        wr16(m, obj + 0x14u, (uint16_t)(d0w + (uint16_t)d2w));
        wr16(m, obj + 0x18u, (uint16_t)(d1w + (uint16_t)d2w));
    }

    sub_12FF36(m, obj);                       /* 0x131554 bsr 0x12FF36 */
    Obj_DrawSprite(m, obj);                   /* 0x131558 jsr 0x1282 */

    /* 0x13155E: subi.w #0x1010,(A0+0x1E) ; bhi 0x131516 (re-arm self) */
    {
        int32_t z = (int32_t)rd16(m, obj + 0x1Eu) - 0x1010;
        wr16(m, obj + 0x1Eu, (uint16_t)z);
        if (z > 0) {
            wr32(m, obj, 0x0013151Eu);
            return 0;
        }
    }

    /* 0x131566: clr.w (A0+6) ; clr.w (A0+0x22) */
    wr16(m, obj + 0x06u, 0u);
    wr16(m, obj + 0x22u, 0u);

    /* 0x131572 */
    wr32(m, obj, 0x0013157Au);
    return 0;
}
