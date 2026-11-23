#include "obj_1077DA.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"
#include "../enemy/obj_E8B72.h"

/* ROM 0x1077DA: the flying enemy that drops in from the right edge. A4 =
 * (obj+0x8) is its parent object. obj+0x6 is its state:
 *
 *   0 = free flight: add the scroll delta (A5+0x4224/0x4228) to the position;
 *       leaving x < -0x30 or y > 0xE8 raises flag 7.3 (it dies)
 *   1 = attached to the parent: pick the parent's mode entry from the table at
 *       0x107A12 (the 68020 `movea.l 0x120(a16,d0.w*4)` at 0x1078F0, base
 *       (0x1078F0+2)+0x120), index the 8-byte {coll.w, angle.w, speed.l}
 *       record at (A4+0x22), steer obj+0x58 towards it (0x0D0C20), copy the
 *       parent position +0x14/+0x1A (up = -y) and add the 0x0D1EBC vector,
 *       take out the 0x1079D2 frame offset of the current anim step, draw
 *       (0x0CFA7C with 0x107A6A) and register the hazard (0x0D0DC2)
 *
 * The NULL entries of the mode table (index 0, and 7 which is its +7 form) are
 * the parent's mode 0: "no handler", never untranslated code.
 *
 * 0x107886 hands a flagged object to the generic dying tick (0x0CEE74), the
 * same call obj_107112.c makes at the end of its dispatch.
 *
 * The sprite def keyed by obj+0x58 is a pair of 1c/flip bytes followed by a
 * sprite record; those 1c/flip bytes are the first row of the 0x179A5C frame
 * script for the current animation op (0x107A6A holds the op and the first
 * row). Delivering them through the frame script is the ROM's own step and is
 * what the captured frames show. */

/* 0x107A12: 4-byte mode-handler pointers, read from ROM (index = parent mode,
 * +7 when bit 7 of the parent's +0x4E is set). */
#define MODE_TABLE 0x00107A12u

/* ROM 0x0D0C20: rotate obj+0x58 (angle.w|step.w) towards D2w by D1w over the
 * 0x168 wrap. Returns the new long; only D0/D1 are updated. */
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

/* ROM 0x0D0DC2: append A0 to the (A5+0x8FE) list, then Coll_AddHazard. */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* ROM 0x107802: the parent is still idle, so pick the flight vector once and
 * put the child into free flight. Both random values come from the 0x0D0E3A
 * LCG, the direction (d0) is drawn first and kept in D1 while the speed is
 * drawn, then 0x0D2D44 scales the direction table by it. */
static void sub_107802(const f3mem_t *m, uint32_t obj)
{
    uint32_t o0, o1;
    uint16_t d1w = (uint16_t)((Rng_Mix1D60(m) & 0x0007u) + 0x001Cu);
    uint16_t d0w = (uint16_t)((Rng_Mix1D60(m) & 0x003Fu) + 0x003Au);

    sub_0D2D44(m, d0w, d1w, &o0, &o1);
    wr32(m, obj + 0x50u, o0);
    wr32(m, obj + 0x54u, o1);
    wr8(m, obj + 0x06u, 0u);
}

/* ROM 0x107836: follow the parent's colour and take over its hit record. */
static void sub_107836(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    uint8_t d0b = rd8(m, a4 + 0x1Du);
    if (d0b != 0u)
        wr8(m, obj + 0x1Du, d0b);

    if (rd8(m, a4 + 0x35u) == 0u && (int32_t)rd32(m, a4 + 0x24u) > 0) {
        /* 0x107852: the parent is hit but has no record; take the child's */
        wr8(m, a4 + 0x35u, rd8(m, obj + 0x35u));
        wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
        wr8(m, obj + 0x35u, 0u);
        if ((int32_t)rd32(m, obj + 0x24u) <= 0) {
            /* 0x10786A */
            wr32(m, a4 + 0x24u, 0u);
            wr32(m, obj + 0x24u, 0x001E0000u);
        }
    }
}

/* ROM 0x10789A: state 0, free flight with a gravity term on +0x54. */
static void state0_fly(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);
    d0 = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);

    if ((int16_t)rd16(m, obj + 0x14u) < (int16_t)0xFFD0 ||
        (int16_t)rd16(m, obj + 0x18u) > (int16_t)0x00E8) {
        /* 0x1078D6 */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u)); /* bset.b #3 */
        return;
    }

    /* 0x1078C6 */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x1078DE: state 1, steer along the parent's mode record and draw. */
static void state1_follow(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    uint16_t d0 = rd8(m, a4 + 0x06u);
    if ((rd8(m, a4 + 0x4Eu) & 0x80u) != 0u)
        d0 = (uint16_t)(d0 + 7u);

    uint32_t a1 = rd32(m, MODE_TABLE + (uint32_t)d0 * 4u);
    if (a1 == 0u)
        return;                              /* mode 0: no handler entry */

    /* 0x1078F8: the {coll.w, angle.w, speed.l} record of the parent's state */
    uint8_t idx = rd8(m, a4 + 0x22u);
    a1 += (uint32_t)idx * 8u;
    wr16(m, obj + 0x42u, rd16(m, a1));
    a1 += 2u;
    uint16_t d2w = rd16(m, a1);
    a1 += 2u;
    uint32_t d1 = rd32(m, a1);

    /* 0x107908 */
    uint32_t d0l = sub_0D0C20(rd32(m, obj + 0x58u), d1, d2w);
    wr32(m, obj + 0x58u, d0l);

    /* 0x107916 */
    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & (uint8_t)~0x08u)); /* bclr.b #3 */
    if ((uint16_t)(d0l >> 16) == d2w)
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x08u));       /* bset.b #3 */

    /* 0x10792A: sit 0x14 right / 4 below the parent's x */
    uint32_t p = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, p + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0014u));
    wr32(m, obj + 0x18u, rd32(m, p + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0004u));

    /* 0x107944: obj += 0x0D1EBC(angle, 0x0D) */
    uint32_t o0, o1;
    sub_0D1EBC(m, rd16(m, obj + 0x58u), 0x000Du, &o0, &o1);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + o0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + o1);

    /* 0x107958: the hit box is the position offset by the same vector */
    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + o0);
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + o1);

    /* 0x10796C: take out the per-step screen offset at 0x1079D2 */
    uint16_t step = rd16(m, 0x000D00B4u + (uint32_t)(int32_t)(int16_t)rd16(m, obj + 0x58u) * 2u);
    uint32_t row = 0x001079D2u + (uint32_t)(int32_t)(int16_t)step;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - rd16(m, row)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - rd16(m, row + 2u)));

    /* 0x10798A: sprite def keyed by the angle */
    sub_0CFA7C(m, obj, 0x00107A6Au, rd16(m, obj + 0x58u));

    /* 0x107998: anchor record -> hit box when the frame has a collision */
    if (rd16(m, obj + 0x42u) != 0u) {
        uint32_t a3 = rd32(m, obj + 0x68u);
        if (a3 != 0u) {
            wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, a3)));
            wr16(m, obj + 0x3Eu, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, a3 + 2u)));
            uint16_t f = rd16(m, obj + 0x4Cu);
            wr32(m, obj + 0x0Cu, rd32(m, a3 + 4u + (uint32_t)f * 4u));
        }
    }

    /* 0x1079CA */
    sub_0D0DC2(m, obj);
}

/* ROM 0x1077DA */
int Obj_1077DA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, 0x5Eu);
    wr32(m, obj + 0x0Cu, 0u);
    wr16(m, obj + 0x4Cu, (uint16_t)((rd16(m, obj + 0x4Cu) + 1u) & 0x0003u));

    if (rd8(m, obj + 0x06u) != 0u) {
        if (rd8(m, a4 + 0x06u) == 0u)
            sub_107802(m, obj);
        else
            sub_107836(m, obj, a4);
    }

    /* 0x107876: `jsr ([0x107892, pc, d0.w*4])` with d0 = obj+0x6 */
    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_fly(m, obj); break;
    case 1u: state1_follow(m, obj, a4); break;
    default: return -1;                      /* only two entries in 0x107892 */
    }

    /* 0x107884: off the playfield -> the generic dying tick (0x0CEE74) */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    /* 0x107892's dispatch returns with the RTS at 0x107890 */
    return 0;
}
