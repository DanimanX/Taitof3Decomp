#include "obj_17DAA0.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17DAA0: follow-the-parent debris tick. A live animation timer (+0x3E
 * >= 0) parks the object at the parent's +0x50/+0x54 plus the +0x16/+0x1A
 * offsets, copies that to the screen position, ticks the animation and centres
 * the sprite before drawing it; the latch stays 0x17DAA0. Once the timer has
 * expired the object is freed. */

/* ROM 0x17DA4A: the live half -- position from the parent, animate, draw */
static int lab_17DA4A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);                /* 0x17DA4A */
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);                          /* 0x17DA5E */
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);                          /* 0x17DA66 */
    wr16(m, obj + 0x18u, d1);

    /* 0x17DA6E: move.w (0x40,A0),D0w ; movea.l (0x4,0x179A5C,D0w),A1 ; jsr */
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;

    /* 0x17DA7E: centre by the frame's own (h, w), each << 3 */
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }
    Obj_DrawSprite(m, obj);                            /* 0x17DA92 */
    wr32(m, obj + 0x00u, 0x0017DAA0u);                 /* 0x17DA98 */
    return 0;
}

int Obj_17DAA0_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17DAA0: tst.w (0x3E,A0) ; bge.b 0x17DA4A */
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0)
        return lab_17DA4A(m, obj);

    /* 0x17DAA6: jsr 0x0105EC */
    Obj_Free(m, obj);
    return 0;
}
