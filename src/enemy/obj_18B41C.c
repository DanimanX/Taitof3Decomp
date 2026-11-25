#include "obj_18B41C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x17B92C: hand the damage banked in +0x24 up to the parent, scaled by
 * the parent's armour class (+0x22); +0x35 == 2 uses the third multiplier. */
static void sub_17B92C(const f3mem_t *m, uint32_t obj)
{
    uint8_t k = rd8(m, obj + 0x35u);
    if (k == 0u)
        return;

    int32_t d0 = (int32_t)rd32(m, obj + 0x24u);
    uint32_t mul = 0;
    if (k == 2u) {
        mul = F3_A5 + 0x4B2Eu;
    } else {
        uint8_t t = rd8(m, rd32(m, obj + 0x36u) + 0x22u);
        if (t == 1u) mul = F3_A5 + 0x4B2Au;
        else if (t == 2u) mul = F3_A5 + 0x4B2Cu;
    }
    if (mul)
        d0 = (int32_t)((uint32_t)(d0 / 100) * (uint32_t)rd16(m, mul));

    uint32_t a4 = rd32(m, obj + 0x08u);
    wr32(m, a4 + 0x24u, rd32(m, a4 + 0x24u) + (uint32_t)d0);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
    wr16(m, a4 + 0x34u, rd16(m, obj + 0x34u));
}

/* ROM 0x18B41C: child part of a multi-part enemy. While the parent is not
 * fading it rides the parent velocity, steps one 0x179A5C frame, draws and is
 * released for the next tick; once the parent raises the fade bit it frees its
 * owner record and itself. */
int Obj_18B41C_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18B41C */
    sub_17B92C(m, obj);

    /* 0x18B422: btst.b #0x2,(0x23,A4) decides alive or gone */
    uint32_t a4 = rd32(m, obj + 0x08u);
    if ((rd8(m, a4 + 0x23u) & 0x04u) != 0u)
        goto free_self;

    /* 0x18B430: parent velocity (+0x50/+0x54) drives the part */
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)));
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x18B454: (0x4,0x179A5C,D0w*8), D0w = +0x40 */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x18B464: centre the frame on its origin */
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t h = rd16(m, a1);
    uint16_t w = rd16(m, a1 + 2u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - (uint16_t)(w << 3)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - (uint16_t)(h << 3)));

    /* 0x18B478: colour from the parent, drop the hit flag, draw */
    wr8(m, obj + 0x1Du, rd8(m, a4 + 0x1Du));
    wr8(m, obj + 0x35u, 0x00u);
    Obj_DrawSprite(m, obj);

    /* 0x18B488: queue on the A5+0x8FE list */
    {
        uint32_t cur = rd32(m, F3_A5 + 0x8FEu);
        wr32(m, cur, obj);
        wr32(m, F3_A5 + 0x8FEu, cur + 4u);
    }

    /* 0x18B492: bftst A5+0x4B3A{0xD:1} -- the sequencer hold flag */
    if ((rd16(m, F3_A5 + 0x4B3Au) & 0x0008u) != 0u)
        goto free_self;

    /* 0x18B498: bra 0x18B414 -> move.l #0x18B41C,(A0) */
    wr32(m, obj + 0x00u, 0x0018B41Cu);
    return 0;

free_self:
    /* 0x18B49C: free the owner record at +0xC, then the object */
    Obj_Free(m, rd32(m, obj + 0x0Cu));
    Obj_Free(m, obj);
    return 0;
}
