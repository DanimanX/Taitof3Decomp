#include "obj_1080F2.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../video/sprite.h"

/* ROM 0x1080F2: an object that rides its parent (A4 = (obj+0x8)).
 *
 * obj+0x6 is the state and selects the handler in the two-entry table at
 * 0x108194 (`jsr ([0x108194, pc, d0.w*4])` at 0x10817E: base = 0x10817E+2+0x14):
 *
 *   0 = 0x10819C: free drift. Add the scroll delta (A5+0x4224/0x4228) to the
 *       position; leaving x < -0x20 or y > 0xE8 raises flag 7.3 (it dies),
 *       otherwise gravity on +0x54 and draw (0x1282)
 *   1 = 0x1081E0: attached. Follow the parent's mode record exactly as
 *       obj_1077DA's state 1 does: pick the record from the table at 0x107FAC
 *       (the 68020 `movea.l 0xFDB8(a16,d0.w*4)` at 0x1081F2, base
 *       0x1081F4-0x2C8), index it by (A4+0x22), then steer obj+0x58 towards
 *       its angle+speed (0x0D0C20), copy the docking position from obj+0x50,
 *       add the 0x0D1EBC vector (current angle, scale 0x0C), store the hit box,
 *       publish it to the parent at +0x60/+0x64, draw the 0x108004 frame set
 *       (0x0CFA7C), take out the frame's anchor words and register the hazard
 *       with 0x0D0DC2.
 *
 * A NULL entry in 0x107FAC (parent mode 0, and mode 7 which is the same in
 * its +7 form) is the ROM's "no handler" mode, not untranslated code. */

/* 0x107FAC: mode-handler records, each an 8-byte row of
 * {collision.w, angle.w, speed.l} starting at the handler address. */
#define MODE_TABLE 0x00107FACu

/* ROM 0x0CFA7C: A1 += (0x0CFA96)[d0w], then the frame header
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

/* ROM 0x0D0C20: rotate (angle.w|step.w) towards D2w by D1w over the 0x168
 * wrap. Returns the new long. */
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
 * through the 0x0D4F26 multiplier cases (plain constants 0..50). */
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
 * indexed by D0w and scaled by D1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);

    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

/* ROM 0x0D0DC2: append A0 to the (A5+0x8FE) list, then Coll_AddHazard. */
static void sub_0D0DC2(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    Coll_AddHazard(m, obj);
}

/* 0x10810A: the parent was in state 0, so pick the drift vector once. The
 * direction is drawn first (kept in D1) and the speed second, then 0x0D2D44
 * scales the direction table by the speed. */
static void sub_10810A(const f3mem_t *m, uint32_t obj)
{
    uint32_t o0, o1;
    uint16_t d1w = (uint16_t)((Obj_Next4Way(m) & 0x0007u) + 0x001Cu);
    uint16_t d0w = (uint16_t)((Obj_Next4Way(m) & 0x003Fu) + 0x003Au);

    sub_0D2D44(m, d0w, d1w, &o0, &o1);
    wr32(m, obj + 0x50u, o0);
    wr32(m, obj + 0x54u, o1);
    wr8(m, obj + 0x06u, 0u);
}

/* 0x10813E: follow the parent's colour and take over its hit record. */
static void sub_10813E(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    wr8(m, obj + 0x1Du, rd8(m, a4 + 0x1Du));

    if (rd8(m, a4 + 0x35u) != 0u || (int32_t)rd32(m, a4 + 0x24u) <= 0)
        return;

    /* 0x108154 */
    wr8(m, a4 + 0x35u, rd8(m, obj + 0x35u));
    wr32(m, a4 + 0x36u, rd32(m, obj + 0x36u));
    wr8(m, obj + 0x35u, 0u);
    if ((int32_t)rd32(m, obj + 0x24u) > 0)
        return;

    /* 0x10816C */
    wr32(m, a4 + 0x24u, 0u);
    wr32(m, obj + 0x24u, 0x001E0000u);
}

/* 0x10819C: state 0, free drift with a gravity term on +0x54. */
static void state0_drift(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);
    d0 = rd32(m, F3_A5 + 0x4228u);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);

    if ((int16_t)rd16(m, obj + 0x14u) < (int16_t)0xFFE0 ||
        (int16_t)rd16(m, obj + 0x18u) > (int16_t)0x00E8) {
        /* 0x1081D8 */
        wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x08u)); /* bset.b #3 */
        return;
    }

    /* 0x1081C8 */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x000036B0u);
    Obj_DrawSprite(m, obj);
}

/* 0x1081E0: state 1, ride the parent's mode record and draw. */
static void state1_attached(const f3mem_t *m, uint32_t obj, uint32_t a4)
{
    uint16_t d0 = rd8(m, a4 + 0x06u);
    if ((rd8(m, a4 + 0x4Eu) & 0x80u) == 0u)
        d0 = (uint16_t)(d0 + 7u);

    uint32_t a1 = rd32(m, MODE_TABLE + (uint32_t)d0 * 4u);
    if (a1 == 0u)
        return;                              /* parent mode 0: no handler */

    /* 0x1081F8: index the {collision.w, angle.w, speed.l} record */
    uint8_t idx = rd8(m, a4 + 0x22u);
    a1 += (uint32_t)idx * 8u;
    wr16(m, obj + 0x42u, rd16(m, a1));
    a1 += 2u;
    uint16_t d2w = rd16(m, a1);
    a1 += 2u;
    uint32_t d1 = rd32(m, a1);

    /* 0x108208: steer the current angle towards the record's */
    uint32_t d0l = sub_0D0C20(rd32(m, obj + 0x58u), d1, d2w);
    wr32(m, obj + 0x58u, d0l);

    /* 0x108216: "lined up" flag 0 of (A4+0x4E) */
    wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) & (uint8_t)~0x01u));
    if ((uint16_t)(d0l >> 16) == d2w)
        wr8(m, a4 + 0x4Eu, (uint8_t)(rd8(m, a4 + 0x4Eu) | 0x01u));

    /* 0x10822A: the docking position lives in the object at obj+0x50 */
    uint32_t p = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, p + 0x44u));
    wr32(m, obj + 0x18u, rd32(m, p + 0x48u));

    /* 0x10823A: obj += 0x0D1EBC(current angle, 0x0C) */
    uint32_t o0, o1;
    sub_0D1EBC(m, rd16(m, obj + 0x58u), 0x000Cu, &o0, &o1);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + o0);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + o1);

    /* 0x10824E: the hit box is the position offset by the same vector */
    wr32(m, obj + 0x44u, rd32(m, obj + 0x14u));
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + o0);
    wr32(m, obj + 0x48u, rd32(m, obj + 0x18u));
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + o1);

    /* 0x108262: publish the hit box to the parent */
    wr32(m, a4 + 0x60u, rd32(m, obj + 0x44u));
    wr32(m, a4 + 0x64u, rd32(m, obj + 0x48u));

    /* 0x10826E: frame set at 0x108004 keyed by the angle */
    sub_0CFA7C(m, obj, 0x00108004u, rd16(m, obj + 0x58u));

    /* 0x10827C: take out the anchor words of the chosen frame */
    uint16_t axw = rd16(m, obj + 0x68u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - axw));
    uint16_t ayw = rd16(m, obj + 0x6Au);
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - ayw));

    /* 0x10828C */
    sub_0D0DC2(m, obj);
}

/* ROM 0x1080F2 */
int Obj_1080F2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1080F2 */
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr8(m, obj + 0x1Du, 0u);

    /* 0x1080FA */
    if (rd8(m, obj + 0x06u) != 0u) {
        if (rd8(m, a4 + 0x06u) == 0u)
            sub_10810A(m, obj);
        else
            sub_10813E(m, obj, a4);
    }

    /* 0x108178: `jsr ([0x108194, pc, d0.w*4])` with d0 = obj+0x6 */
    switch (rd8(m, obj + 0x06u)) {
    case 0u: state0_drift(m, obj); break;
    case 1u: state1_attached(m, obj, a4); break;
    default: return -1;                      /* only two entries in 0x108194 */
    }

    /* 0x108186: off the playfield -> the generic dying tick (0x0CEE74) */
    if ((rd8(m, obj + 0x07u) & 0x08u) != 0u) {
        Obj_StartDying(m, obj);
        return 0;
    }

    return 0;
}
