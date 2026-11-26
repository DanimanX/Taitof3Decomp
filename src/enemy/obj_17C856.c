#include "obj_17C856.h"
#include "../objects/obj_anim.h"
#include "../video/sprite.h"

/* ROM 0x17C856: init of a parent-anchored decoration. Takes the parent's
 * +0x50/+0x54, the frame list 0x17C8E0 (first word = 0x179A5C index into
 * +0x40, rest into +0x3A), runs that entry's init and its first frame tick,
 * centres on the frame size, draws, and arms 0x17C8CC. */
int Obj_17C856_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));

    uint16_t d0 = rd16(m, 0x0017C8E0u);
    wr16(m, obj + 0x40u, d0);
    wr32(m, obj + 0x3Au, 0x0017C8E2u);
    if (Obj_AnimInit(m, obj, d0) < 0)               /* (0x179A5C,D0*8).init */
        return -1;
    wr16(m, obj + 0x1Eu, 0x0000u);
    wr16(m, obj + 0x1Cu, 0x0008u);

    return Obj_17C88E_Body(m, obj);                 /* 0x17C88E */
}

/* ROM 0x17C88E: centre on +0x50/+0x54, animation tick, frame-size centre,
 * draw, arm 0x17C8CC. */
int Obj_17C88E_Body(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    if (Obj_AnimTick(m, obj, rd16(m, obj + 0x40u)) < 0)
        return -1;
    uint32_t a1 = rd32(m, obj + 0x10u);
    uint16_t d1w = (uint16_t)(rd16(m, a1) << 3);
    uint16_t d0w = (uint16_t)(rd16(m, a1 + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w));
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C8CCu);
    return 0;
}
