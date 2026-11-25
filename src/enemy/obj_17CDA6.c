#include "obj_17CDA6.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x17CDA6: the grown-out tail of the 0x17CDxx family. While +0x1E is
 * zero it is the 0x17CD28 body (fall-through at 0x17CDA6); on the tick +0x1E
 * becomes non-zero it runs 0x17CDAE once and hands off to 0x17CE04. */

/* ROM 0x17CDAE: move on top of the parent's +0x50/+0x54, animate, centre, draw */
static void lab_17CDAE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
    uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
    wr16(m, obj + 0x50u, d0);
    wr16(m, obj + 0x54u, d1);
    wr16(m, obj + 0x14u, d0);
    wr16(m, obj + 0x18u, d1);

    Obj_AnimTick(m, obj, rd16(m, obj + 0x40u));  /* 0x17CDD2 jsr (A1) */

    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1c = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0c = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0c));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1c));
    }
    Obj_DrawSprite(m, obj);                     /* 0x17CDF6 */
    wr32(m, obj + 0x00u, 0x0017CE04u);          /* 0x17CDFC */
}

int Obj_17CDA6_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17CDA6: tst.b (0x1E,A0); bne 0x17CD28 -- still growing: the 0x17CD28
     * body (move/animate/centre, +0x1E/+0x1F += 0xF0). Its animation dispatch
     * has no translation here, so the tick is genuinely unimplemented. */
    if (rd8(m, obj + 0x1Eu) != 0u)
        return -1;

    /* 0x17CDAE fall-through: the 0x17CD66 tail installs 0x17CDA6 and returns;
     * +0x1E is still zero on this tick. */
    lab_17CDAE(m, obj);
    return 0;
}
