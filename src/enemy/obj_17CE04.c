#include "obj_17CE04.h"
#include "../objects/obj_common.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17CE04: the countdown half of the 0x17CDxx family. +0x06 counts down;
 * while it is positive the tick is the 0x17CDAE body, once it hits zero the
 * full 0x17CE0A body runs (grown-out zoom step) and 0x17CE88 takes over. */

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

int Obj_17CE04_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x17CE04: subq.w #1,(0x6,A0); bgt 0x17CDAE */
    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) - 1u));
    if ((int16_t)rd16(m, obj + 0x06u) > 0) {
        lab_17CDAE(m, obj);
        return 0;
    }

    return Obj_17CE0A_Body(m, obj);
}

/* ROM 0x17CE0A: grow-out step -- follow the parent, animate, centre, grow
 * +0x1E/+0x1F by 0x20, re-centre by the scaled size, draw, arm 0x17CE88. */
int Obj_17CE0A_Body(const f3mem_t *m, uint32_t obj)
{
    /* 0x17CE0A: parent-relative move, animation tick, sprite centre */
    {
        uint32_t a4 = rd32(m, obj + 0x08u);
        uint16_t d0 = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u));
        uint16_t d1 = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au));
        wr16(m, obj + 0x50u, d0);
        wr16(m, obj + 0x54u, d1);
        wr16(m, obj + 0x14u, d0);
        wr16(m, obj + 0x18u, d1);
    }
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)   /* 0x17CE2E */
        return -1;
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1c = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0c = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0c));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1c));
    }

    /* 0x17CE52: +0x1E/+0x1F += 0x20, both offsets += (+0x1F * word@+0x10 << 4) >> 9 */
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) + 0x20u));
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) + 0x20u));
    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1c = (uint16_t)(rd16(m, a1) << 4);
        uint16_t d0c = (uint16_t)(((uint16_t)rd8(m, obj + 0x1Fu) * d1c) >> 8);
        d0c = (uint16_t)(d0c >> 1);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0c));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d0c));
    }

    Obj_DrawSprite(m, obj);                     /* 0x17CE7A jsr 0x001282 */
    wr32(m, obj + 0x00u, 0x0017CE88u);          /* 0x17CE80 */
    return 0;
}
