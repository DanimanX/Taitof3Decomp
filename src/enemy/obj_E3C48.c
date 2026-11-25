#include "obj_E3C48.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D1EBC / 0x0D2D44: (D0, D1) = per-angle 16.16 tables scaled by the
 * index of the 0x0D4F26 multiply table (block k multiplies by k). */
static void angle_scale(const f3mem_t *m, uint32_t t0, uint32_t t1,
                        uint16_t ang, uint16_t k, uint32_t *d0, uint32_t *d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)ang * 4u;
    *d0 = rd32(m, t0 + off) * (uint32_t)k;
    *d1 = rd32(m, t1 + off) * (uint32_t)k;
}

/* ROM 0x0E3C48: init of the 0x0E3CDA shot. Inherits the heading (+0x58)
 * from its parent and its position from the grandparent (+8, +5), takes the
 * 0x0E3D8A stats and a frame set chosen by +0x10 (pointer table 0x0E3DAA),
 * pushes its anchor 0x12 units along the +0x68 angle and gets a 0x23-unit
 * velocity along it. */
int Obj_E3C48_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x58u, rd32(m, a1 + 0x58u));
    a1 = rd32(m, a1 + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 8u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 5u));

    Obj_0CEDE4(m, obj, 0x000E3D8Au);               /* lea (0x11E,PC) */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u));

    /* 0x0E3C80: movea.l (0x128,PC,D0.w*4),A1 -- full-format ext at 0x0E3C82 */
    a1 = rd32(m, 0x000E3DAAu + (uint32_t)rd8(m, obj + 0x10u) * 4u);
    Obj_0CFA7C(m, obj, a1, rd16(m, obj + 0x58u));

    uint32_t d0, d1;
    angle_scale(m, 0x000D1EDCu, 0x000D2044u, rd16(m, obj + 0x68u), 0x12u, &d0, &d1);
    wr32(m, obj + 0x3Au, rd32(m, obj + 0x3Au) + d0);
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) + d1);
    angle_scale(m, 0x000D2D64u, 0x000D2ECCu, rd16(m, obj + 0x68u), 0x23u, &d0, &d1);
    wr32(m, obj + 0x50u, d0);
    wr32(m, obj + 0x54u, d1);

    wr32(m, obj + 0x14u, rd32(m, obj + 0x3Au));
    wr32(m, obj + 0x18u, rd32(m, obj + 0x3Eu));
    Obj_DrawSprite(m, obj);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & (uint8_t)~0x02u));
    wr32(m, obj + 0x00u, 0x000E3CDAu);
    return 0;
}
