#include "obj_18B4E8.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x18B4E0: move.l #0x18B4E8,(A0); rts */
static void restart(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x0018B4E8u);
}

/* ROM 0x18B4E8: pick the child's per-frame velocity from the 6-entry table at
 * 0x18B56C keyed by the parent sprite id, integrate the parent position and run
 * one frame-animation step. */
int Obj_18B4E8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x18B4E8: movea.l (0x8,A0),A1; move.l (0x10,A1),D0 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    uint32_t d0 = rd32(m, a1 + 0x10u);

    /* 0x18B4F0: A1 = 0x18B56C, D1w = 5; loop cmp.l (A1),D0 / beq out /
     * lea (0x8,A1),A1 / dbf -- falling off the end leaves A1 past record 5 */
    uint32_t tab = 0x0018B56Cu;
    for (uint16_t d1 = 5u;; d1--) {
        if (rd32(m, tab) == d0) {
            a1 = tab;
            break;
        }
        tab += 8u;
        a1 = tab;
        if (d1 == 0u)
            break;
    }

    /* 0x18B508: lea (0x4,A1),A1; {w16, w16} -> +0x16 / +0x1A */
    a1 += 4u;
    wr16(m, obj + 0x16u, rd16(m, a1));
    wr16(m, obj + 0x1Au, rd16(m, a1 + 2u));

    /* 0x18B514: velocity integration on the parent position */
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0w = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1w = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0w);
    wr16(m, obj + 0x54u, d1w);
    wr16(m, obj + 0x14u, d0w);
    wr16(m, obj + 0x18u, d1w);

    /* 0x18B538: jsr ((0x4,0x179A5C,D0w*8)) -- the id in +0x40 */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x18B548: centre the frame on its origin: (A1)+ then (A1)+, both << 3 */
    a1 = rd32(m, obj + 0x10u);
    uint16_t s1 = (uint16_t)(rd16(m, a1) << 3);        /* move.w (A1)+,D1w; asl.w #3 */
    uint16_t s0 = (uint16_t)(rd16(m, a1 + 2u) << 3);   /* move.w (A1)+,D0w; asl.w #3 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - s0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - s1));

    /* 0x18B55C: move.b (0x1D,A4),(0x1D,A0) -- the colour override copied down
     * from the parent (A4 still holds the +0x8 link) */
    wr8(m, obj + 0x1Du, rd8(m, a4 + 0x1Du));

    Obj_DrawSprite(m, obj);                            /* 0x18B562 jsr 0x1282 */

    restart(m, obj);                                   /* 0x18B568 bra 0x18B4E0 */
    return 0;
}
