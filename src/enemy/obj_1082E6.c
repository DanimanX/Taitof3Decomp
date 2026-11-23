#include "obj_1082E6.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x1082E6: the sprite object latched onto a parent object (A4 =
 * (obj+0x8)): 0x0CFA7C sets the sprite from the first `movea.l (0x8,A4),A4`
 * when the parent's slot is dead (A4+0x6 = 0), otherwise it copies the
 * parent's colour (A4+0x1D) and lets 0x0CFA7C re-arm the sprite every frame.
 * obj+0x6 selects the handler from the table at 0x10835C:
 *
 *   0 = 0x108364: follow the scroll delta (A5+0x4224/0x4228), and while
 *       x.w >= -0x20 and y.w <= 0xE8 add 0x36B0 to the "height" field +0x54
 *       and draw; leaving that window raises flag 7.3 instead
 *   1 = 0x1083A8: latch onto the sprite object two links down from +0x54,
 *       take its position and frame, steer +0x14/+0x18 by the parent angle
 *       (A4+0x8 - 0x30, wrapped over 0x168) scaled by 0x0A (0x0D1EBC), lift
 *       by -0x1A/-5, then draw from the frame table at 0x108420
 *
 * Only those two entries exist and the capture only takes state 0. */

/* ROM 0x0D0E3A: 4-way interleaved LCG (x' = 5x + 1) on A5+0x1D60; returns the
 * high byte of the result (D0.w >> 8). Reached only by the parent-recycled
 * path at 0x108300. */
static uint16_t sub_0D0E3A(const f3mem_t *m)
{
    uint16_t d0 = rd16(m, F3_A5 + 0x1D60u);
    uint16_t d1 = (uint16_t)(d0 << 2);
    d0 = (uint16_t)(d0 + d1 + 1u);
    wr16(m, F3_A5 + 0x1D60u, d0);
    return (uint16_t)(d0 >> 8);
}

/* ROM 0x0D2D44: (D0, D1) = per-angle speed tables {0x0D2D64, 0x0D2ECCu}
 * indexed by D0w and scaled by D1w (the same 0x0D4F26 multiplier). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);

    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0D1EBC: (D0, D1) = {0x0D1EDC, 0x0D2044}[angle.w] scaled by D1w
 * through the 0x0D4F26 cases, which are plain constants 0..50. */
static void sub_0D1EBC(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D1EDCu + off);
    uint32_t d1 = rd32(m, 0x000D2044u + off);

    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0CFA7C: A1 += (0x0CFA96)[d0w], then the 0x0CFA86 sprite header
 * (flip byte, sprite def, anchor pointer). */
static void sub_0CFA7C(const f3mem_t *m, uint32_t obj, uint32_t a1, uint16_t d0w)
{
    uint16_t off = rd16(m, 0x000CFA96u + (uint32_t)((int32_t)(int16_t)d0w * 2));
    a1 += (uint32_t)(int32_t)(int16_t)off;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
}

/* ROM 0x108364: state 0, follow the scroll delta on the "height" field. */
static void state0_follow(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);
    d0 = rd32(m, obj + 0x54u) - rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);

    if ((int16_t)rd16(m, obj + 0x14u) < (int16_t)0xFFE0 ||
        (int16_t)rd16(m, obj + 0x18u) > (int16_t)0x00E8) {
        /* 0x1083A0: off the playfield, bset.b #3 on the flag byte at 7 */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u));
        return;
    }

    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
    Obj_DrawSprite(m, obj);                  /* 0x108398: jsr 0x001282 */
}

/* ROM 0x1083A8: state 1, follow the sprite object two links down from +0x54. */
static void state1_attach(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    uint32_t a1 = rd32(m, obj + 0x54u);
    a1 = rd32(m, a1 + 0x54u);
    a1 = rd32(m, a1 + 0x54u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x48u));
    wr16(m, obj + 0x58u, rd16(m, a1 + 0x58u));

    uint16_t d0w = (uint16_t)(rd16(m, a4 + 0x08u) - 0x0030u);
    if ((int16_t)d0w < 0)
        d0w = (uint16_t)(d0w + 0x0168u);

    uint32_t o0, o1;
    sub_0D1EBC(m, d0w, 0x000Au, &o0, &o1);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + o0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + o1);

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Au));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 5u));

    /* 0x1083F0: lea.l 0x108420(pc),A1; d0 = obj+0x58 -> 0x0CFA7C */
    sub_0CFA7C(m, obj, 0x00108420u, rd16(m, obj + 0x58u));
}

/* ROM 0x1082E6: called with A0 = obj. Returns 0, or -1 on a ROM path the
 * capture does not exercise (no side effects of that path applied). */
int Obj_1082E6_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, 0x5Eu);

    if (rd8(m, obj + 0x06u) != 0u) {
        if (rd8(m, a4 + 0x06u) != 0u) {
            /* 0x108334: the parent is alive, take over its colour */
            uint8_t d0b = rd8(m, a4 + 0x1Du);
            if (d0b != 0u)
                wr8(m, obj + 0x1Du, d0b);
        } else {
            /* 0x108300: the parent's slot is idle, so pick the follow vector
             * once (direction drawn first, kept in D1 while the speed is
             * drawn) and hand the sprite back to the frame code. */
            uint32_t o0, o1;
            uint16_t d1w = (uint16_t)((sub_0D0E3A(m) & 0x0007u) + 0x001Cu);
            uint16_t d0w = (uint16_t)((sub_0D0E3A(m) & 0x003Fu) + 0x003Au);

            sub_0D2D44(m, d0w, d1w, &o0, &o1);
            wr32(m, obj + 0x50u, o0);
            wr32(m, obj + 0x54u, o1);
            wr8(m, obj + 0x06u, 0u);
        }
    }

    /* 0x108346: `jsr ([0x10835C, pc, d0.w*4])` with d0 = obj+0x6 */
    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_follow(m, obj); break;
    case 1u: state1_attach(m, obj, a4); break;
    default: return -1;                      /* only two entries in 0x10835C */
    }

    /* 0x10834E: btst.b #3, obj+7 -> the generic dying tick at 0x0CEE74 */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u)
        Obj_StartDying(m, obj);

    return 0;
}
