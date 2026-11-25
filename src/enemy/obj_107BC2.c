#include "obj_107BC2.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D0C20: step 16.16 angle d0 toward target d2w by step d1 (mod 0x168). */
static uint32_t sub_0D0C20(uint32_t d0, uint32_t d1, uint16_t d2w)
{
    uint16_t d0w = (uint16_t)(d0 >> 16);
    uint16_t d1w = (uint16_t)(d1 >> 16);
    uint16_t d3w = (uint16_t)(d2w - d0w);

    if ((int16_t)d3w < 0)
        d3w = (uint16_t)(d3w + 0x0168u);

    if (d3w > 0x00B4u) {
        d3w = (uint16_t)(0x0168u - d3w);
        if (d3w <= d1w)
            return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
        d0 -= d1;
        if ((int32_t)d0 < 0)
            d0 += 0x01680000u;
        return d0;
    }

    if (d3w <= d1w)
        return ((uint32_t)d2w << 16) | (d0 & 0xFFFFu);
    d0 += d1;
    if (d0 >= 0x01680000u)
        d0 -= 0x01680000u;
    return d0;
}

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

/* ROM 0x107C7A: state 0 (detached / falling offscreen). */
static void state0_fall(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x14u,
         rd32(m, obj + 0x14u) + (rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u)));
    wr32(m, obj + 0x18u,
         rd32(m, obj + 0x18u) + (rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u)));

    if ((int16_t)rd16(m, obj + 0x14u) < -0x30 ||
        (int16_t)rd16(m, obj + 0x18u) > 0x00E8) {
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        return;
    }

    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x107CBE: state 1 (aimed turret / arm attached to parent). */
static void state1_track(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    uint16_t d0w = rd8(m, a4 + 0x06u);
    if ((rd8(m, a4 + 0x4Eu) & 0x80u) == 0u)
        d0w = (uint16_t)(d0w + 7u);

    uint32_t a1 = rd32(m, 0x00107A12u + (uint32_t)d0w * 4u);
    a1 += (uint32_t)rd8(m, a4 + 0x22u) * 8u;

    wr16(m, obj + 0x42u, rd16(m, a1));
    uint16_t d2w = rd16(m, a1 + 2u);
    uint32_t d1 = rd32(m, a1 + 4u);
    uint32_t d0 = sub_0D0C20(rd32(m, obj + 0x58u), d1, d2w);
    wr32(m, obj + 0x58u, d0);

    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & ~0x02u));
    if ((uint16_t)(d0 >> 16) == d2w)
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x02u));

    uint32_t anchor = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, anchor + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, anchor + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 4u));

    uint16_t ang = rd16(m, obj + 0x58u);
    uint32_t ox, oy;
    sub_0D1EBC(m, ang, 0x000Du, &ox, &oy);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + ox);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + oy);
    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u) + ox);
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u) + oy);

    uint16_t frame_off = rd16(m, 0x000D00B4u + (uint32_t)ang * 2u);
    uint32_t off_rec = 0x001079D2u + (uint32_t)(int32_t)(int16_t)frame_off;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, off_rec)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, off_rec + 2u)));

    Obj_0CFA7C(m, obj, 0x00107A6Au, ang);

    if (rd16(m, obj + 0x42u) != 0u) {
        uint32_t ptr = rd32(m, obj + 0x68u);
        if (ptr != 0u) {
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, ptr)));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, ptr + 2u)));
            wr32(m, obj + 0x0Cu,
                 rd32(m, ptr + 4u + (uint32_t)rd16(m, obj + 0x4Cu) * 4u));
        }
    }

    Obj_0D0DC2(m, obj);
}

/* ROM 0x107BC2 */
int Obj_107BC2_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);

    wr8(m, obj + 0x1Du, 0u);
    wr32(m, obj + 0x0Cu, 0u);
    wr16(m, obj + 0x4Cu, (uint16_t)((rd16(m, obj + 0x4Cu) + 1u) & 3u));

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
            if (rd8(m, a4 + 0x35u) == 0u && (int32_t)rd32(m, a4 + 0x24u) > 0) {
                wr8(m, a4 + 0x35u, rd8(m, obj + 0x35u));
                wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
                wr8(m, obj + 0x35u, 0u);
                if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
                    wr32(m, a4 + 0x24u, 0u);
                    wr32(m, obj + 0x24u, 0x001E0000u);
                }
            }
        }
    }

    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_fall(m, obj); break;
    case 1u: state1_track(m, obj, a4); break;
    default: break;
    }

    if (rd8(m, obj + 0x07u) & 0x08u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    return 0;
}
