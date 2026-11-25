#include "obj_108512.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[angle.w] scaled by D1w. */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);
    uint32_t d1 = rd32(m, 0x000D2044u + off);

    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables {0x0D2D64, 0x0D2ECC}
 * indexed by D0w and scaled by D1w (0x0D4F26 multiplier). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);

    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x108588: state 0 (detached / falling offscreen). */
static void state0_fall(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u,
         rd32(m, obj + 0x14u) + (rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u)));
    wr32(m, obj + 0x18u,
         rd32(m, obj + 0x18u) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));

    if ((int16_t)rd16(m, obj + 0x14u) < -0x20 ||
        (int16_t)rd16(m, obj + 0x18u) > 0x00E8) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        return;
    }

    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x1085CC: state 1 (attached tail/thruster segment). */
static void state1_attach(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    uint32_t a1 = rd32(m, obj + 0x54u);
    a1 = rd32(m, a1 + 0x54u);
    a1 = rd32(m, a1 + 0x54u);

    wr32(m, obj + 0x14u, rd32(m, a1 + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x48u));
    wr16(m, obj + 0x58u, rd16(m, a1 + 0x58u));

    int16_t d0w = (int16_t)(rd16(m, a4 + 0x08u) - 0x30u);
    if (d0w < 0)
        d0w = (int16_t)(d0w + 0x0168);

    uint32_t ox, oy;
    sub_0D1EBC(m, (uint16_t)d0w, 0x000Au, &ox, &oy);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + ox);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + oy);

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x1Au));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));

    Obj_0CFA7C(m, obj, 0x00108420u, rd16(m, obj + 0x58u));
}

/* ROM 0x108512 */
int Obj_108512_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);

    wr8(m, obj + 0x1Du, 0u);
    if (rd8(m, obj + 0x06u) != 0u) {
        if (rd8(m, a4 + 0x06u) == 0u) {
            uint16_t d1w = (uint16_t)((Obj_Next4Way(m) & 0x07u) + 0x1Cu);
            uint16_t d0w = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) + 0x3Au);
            uint32_t vx, vy;
            sub_0D2D44(m, d0w, d1w, &vx, &vy);
            wr32(m, obj + 0x50u, vx);
            wr32(m, obj + 0x54u, vy);
            wr8(m, obj + 0x06u, 0u);
        } else {
            wr8(m, obj + 0x1Du, rd8(m, a4 + 0x1Du));
        }
    }

    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_fall(m, obj); break;
    case 1u: state1_attach(m, obj, a4); break;
    default: break;
    }

    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    return 0;
}
