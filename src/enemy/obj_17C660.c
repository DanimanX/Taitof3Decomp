#include "obj_17C660.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x17C660: `bra.b 0x17C614` (60 B2). The tick is only the 0x17C614 body;
 * the +0x34 bit-1 dispatch at 0x17C59A (score credit / frame-list re-pick, or
 * the 0x17C662 fade) is a different entry and is not reached from here. */

/* ROM 0x17C614: script step, animation, the frame-centre correction, the
 * offscreen test and then the draw call. */
static int sub_17C614(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (Obj_Script10(m, obj) < 0)
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));

    /* 0x17C62A: move.w (0x40,A0),D0 ; movea.l (0x179A5C+4,D0.w*8),A1 ; jsr (A1) */
    if (Obj_AnimTick(m, obj, idx) < 0)
        return -1;

    {
        uint32_t a1 = rd32(m, obj + 0x10u);
        uint16_t d1 = (uint16_t)(rd16(m, a1) << 3);
        uint16_t d0 = (uint16_t)(rd16(m, a1 + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1));
    }

    /* 0x17C64A: a negative animation timer means the object goes away */
    if ((int16_t)rd16(m, obj + 0x3Eu) < 0) {
        Obj_Free(m, obj);
        return 0;
    }
    Obj_DrawSprite(m, obj);
    wr32(m, obj + 0x00u, 0x0017C660u);
    return 0;
}

int Obj_17C660_Tick(const f3mem_t *m, uint32_t obj)
{
    return sub_17C614(m, obj, rd16(m, obj + 0x40u));
}
