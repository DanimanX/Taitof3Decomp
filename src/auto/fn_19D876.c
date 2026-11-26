/* ROM 0x19D876: boss segment / articulated tentacle/limb tick (A0 = object).
 * If A5+0x4B3A{14:1} is set, clears +0x24. If +0x24 <= 0, copies (+0x14,+0x18)
 * + (0x10, 0x10) to (+0x50, +0x54) and runs death explosion sequence 0x19E540
 * (increments A5+0x4BC2, awards 0x7530 via 0x17B02E, +0x35 score bonus
 * 0x12C5F to A5+0x886, sets A5+0x4B26{2:1}, spawns 0x179F6C, sets script
 * 0x19E65E, ticks 0x19E5CC with 0x178982/0x179A5C/0x1282/0x179F26 or exits
 * via 0x17AAC4).
 * When +0x24 > 0:
 *   - If A5+0x4B3A{10:1} == 0: syncs (+0x50,+0x54,+0x20) from parent via
 *     0x19DD78, ticks anim, centers sprite, registers hitboxes on A5+0x8FE/
 *     0x906 if !A5+0x4B3A{15:1}, jitters on +0x35, runs 0x19DC78 periodic shot
 *     (0x18339C/0x183584), draws sprite, sets +0x34 = 0x0A, re-arms 0x19D876.
 *   - Else: calls 0x19DD5A if +0x44 == 0x19DD3C, steps script10 (0x178982),
 *     saves +0x20, runs head draw 0x19D9D8 (anim 2 / 0x19DF76) and 16-segment
 *     trail draw/step 0x19DAAA (cmp2.w [-21, 21] at 0x19D7CE, anim 10 /
 *     0x19DEB4, hit jitter/flash, 0x1282, A5+0x906/0x8FE), restores +0x20, sets
 *     +0x34 = 0x14, increments +0x62 mod 16, and re-arms 0x19D876. */
#include "fn_auto.h"
#include "../audio/sound_cmd.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

static int16_t abs16(int16_t v)
{
    return v < 0 ? (int16_t)(0u - (uint16_t)v) : v;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* Center sprite position around (+0x14, +0x18) by frame dimensions * 8. */
static void center_sprite(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t dy = (int16_t)(rd16(m, a1) << 3);
    int16_t dx = (int16_t)(rd16(m, a1 + 2u) << 3);
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - dx));
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - dy));
}

/* ROM 0x1798D4 (0x179A5C[10].tick): 32-dir 6-byte {spr.l, 1c.w} lookup;
 * shared Obj_AnimTick(..., 10) omits the +0x1C byte write, so keep local. */
static int anim_tick_19D876(const f3mem_t *m, uint32_t obj, uint16_t idx)
{
    if (idx == 10u) {
        uint8_t ang = rd8(m, obj + 0x20u);
        uint16_t d0 = (uint16_t)((uint8_t)(ang + 4u) >> 3);
        uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * 6u & 0xFFFFu);
        wr32(m, obj + 0x10u, rd32(m, a1));
        wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1 + 4u));
        return 0;
    }
    return Obj_AnimTick(m, obj, idx);
}

/* ROM 0x17B02E: resolve score receiver from +0x35. */
static uint32_t sub_17B02E(const f3mem_t *m, uint32_t obj)
{
    uint8_t mode = rd8(m, obj + 0x35u);
    if (mode == 1u)
        return rd32(m, rd32(m, obj + 0x36u) + 0x08u);
    if (mode == 2u)
        return rd32(m, obj + 0x36u);
    return 0xFFFFFFFFu;
}

/* ROM 0x19DD5A: copy parent (+0x50,+0x54) + (+0x64,+0x66) to target (+0x4C,+0x4E). */
static void sub_19DD5A(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x4Cu, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x64u)));
    wr16(m, obj + 0x4Eu, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x66u)));
}

/* ROM 0x19DD78: copy parent (+0x50,+0x54) + (+0x64,+0x66) to (+0x50,+0x54) & +0x68->+0x20. */
static void sub_19DD78(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x64u)));
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x66u)));
    wr16(m, obj + 0x20u, rd16(m, obj + 0x68u));
}

/* ROM 0x178FCE: steer toward (+0x4C, +0x4E) within |+0x42.b| threshold. */
static void scr_178FCE(const f3mem_t *m, uint32_t obj)
{
    int16_t d2 = (int16_t)(int8_t)rd8(m, obj + 0x42u);
    if (d2 < 0) {
        wr16(m, obj + 0x42u, 0u);
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    int16_t ex = (int16_t)(rd16(m, obj + 0x4Cu) - rd16(m, obj + 0x50u));
    int16_t ey = (int16_t)(rd16(m, obj + 0x4Eu) - rd16(m, obj + 0x54u));
    if (!(abs16(ex) > d2 || abs16(ey) > d2)) {
        wr16(m, obj + 0x48u, 0u);
        return;
    }

    uint16_t ang = Obj_Atan2(m, (uint16_t)ex, (uint16_t)ey);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t spd = rd16(m, a1 + 8u);
    int8_t diff = (int8_t)(uint8_t)((uint8_t)ang - rd8(m, obj + 0x20u));
    if (diff == 0)
        spd = 0u;
    else if (diff < 0)
        spd = (uint16_t)(0u - spd);

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + spd));

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178ECC: aim heading toward (+0x4C)->(+0x14+0x18, +0x18+0x10) and step. */
static void scr_178ECC(const f3mem_t *m, uint32_t obj)
{
    int32_t tgt = (int32_t)rd32(m, obj + 0x4Cu);
    if (tgt <= 0)
        return;
    uint32_t t = (uint32_t)tgt;
    uint16_t ex = (uint16_t)(rd16(m, t + 0x14u) + 0x0018u - rd16(m, obj + 0x50u));
    uint16_t ey = (uint16_t)(rd16(m, t + 0x18u) + 0x0010u - rd16(m, obj + 0x54u));
    uint16_t ang = Obj_Atan2(m, ex, ey);
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t step = rd16(m, a1 + 6u);
    uint16_t jitter = rd16(m, a1 + 8u);
    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + step));
    uint8_t b = (uint8_t)(((uint8_t)ang & (uint8_t)jitter)
                          + (uint8_t)(int8_t)(int16_t)(jitter >> 8));
    wr8(m, obj + 0x20u, b);

    int16_t mag = (int16_t)(uint16_t)(rd16(m, obj + 0x42u) + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(rd16(m, obj + 0x20u) >> 8);
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178CCE: step Cartesian (+0x42, +0x5C) velocity components & update +0x20 angle. */
static void scr_178CCE(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t vxw = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint16_t vyw = (uint16_t)(rd16(m, obj + 0x5Cu) + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, vxw);
    wr16(m, obj + 0x5Cu, vyw);

    uint32_t dx = (uint32_t)((int32_t)(int16_t)(uint16_t)(vxw + 0x20u) << 8) & 0xFFFFC000u;
    uint32_t dy = (uint32_t)((int32_t)(int16_t)(uint16_t)(vyw + 0x20u) << 8) & 0xFFFFC000u;
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + dx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + dy);

    uint16_t ang = Obj_Atan2(m, rd16(m, obj + 0x42u), rd16(m, obj + 0x5Cu));
    wr8(m, obj + 0x20u, (uint8_t)ang);
}

/* ROM 0x178C06: advance along 0x27E40 sine/cosine table. */
static void scr_178C06(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x44u);
    uint16_t spd = (uint16_t)(rd16(m, obj + 0x42u) + rd16(m, a1 + 6u));
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, a1 + 8u));
    wr16(m, obj + 0x42u, spd);
    wr16(m, obj + 0x20u, ang);

    int16_t mag = (int16_t)(uint16_t)(spd + 0x20u) >> 6;
    uint16_t idx = (uint16_t)(ang >> 8);
    int32_t vx = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)idx * 2u);
    int32_t vy = (int32_t)mag * (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u);
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + (uint32_t)vx);
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + (uint32_t)vy);
}

/* ROM 0x178982 step supporting both shared script10 ops and 0x19D876 local callbacks. */
static int script10_step_19D876(const f3mem_t *m, uint32_t obj)
{
    for (;;) {
        uint32_t a1 = rd32(m, obj + 0x44u);
        uint32_t fn = rd32(m, a1);

        if (fn == 0u)
            goto loop_marker;

        if (fn == 0x00178B20u) {
            Obj_Script10_SetHeading(m, obj);
        } else if (fn == 0x00178B2Cu) {
            wr16(m, obj + 0x5Cu, rd16(m, a1 + 8u));
        } else if (fn == 0x00178BB4u) {
            wr32(m, obj + 0x4Cu, rd32(m, F3_A5 + 0x1D28u));
        } else if (fn == 0x00178C06u) {
            scr_178C06(m, obj);
        } else if (fn == 0x00178CCEu) {
            scr_178CCE(m, obj);
        } else if (fn == 0x00178ECCu) {
            scr_178ECC(m, obj);
        } else if (fn == 0x00178FCEu) {
            scr_178FCE(m, obj);
        } else if (fn == 0x00179276u) {
            int16_t c0 = (int16_t)rd16(m, a1 + 6u);
            if (c0 >= 0)
                Sound_PlayCmd(m, Sound_PreCmdHook(m, c0));
            int16_t c1 = (int16_t)rd16(m, a1 + 8u);
            if (c1 >= 0)
                Sound_PlayCmd(m, Sound_PreCmdHook(m, c1));
        } else if (fn == 0x0019DD5Au) {
            sub_19DD5A(m, obj);
        } else if (fn == 0x0019DD78u) {
            sub_19DD78(m, obj);
        } else {
            return Obj_Script10(m, obj);
        }

        int16_t n = (int16_t)rd16(m, obj + 0x48u);
        if (n < 0)
            return 0;
        if (n > 0) {
            n = (int16_t)(n - 1);
            wr16(m, obj + 0x48u, (uint16_t)n);
            if (n > 0)
                return 0;
            a1 = rd32(m, obj + 0x44u) + 0x0Au;
            if ((int32_t)rd32(m, a1) > 0) {
                wr32(m, obj + 0x44u, a1);
                wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                return 0;
            }
            goto loop_marker;
        }
        a1 = rd32(m, obj + 0x44u) + 0x0Au;
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
    }

loop_marker: {
        uint32_t a1 = rd32(m, obj + 0x44u);
        int16_t c = (int16_t)rd16(m, obj + 0x4Au);
        if (c >= 0) {
            if (c == 0)
                wr16(m, obj + 0x4Au, rd16(m, a1 + 4u));
            c = (int16_t)(rd16(m, obj + 0x4Au) - 1u);
            wr16(m, obj + 0x4Au, (uint16_t)c);
            if (c == 0) {
                a1 += 0x0Au;
                wr32(m, obj + 0x44u, a1);
                wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
                return 0;
            }
        }
        a1 = rd32(m, a1 + 6u);
        wr32(m, obj + 0x44u, a1);
        wr16(m, obj + 0x48u, rd16(m, a1 + 4u));
        return 0;
    }
}

/* ROM 0x19DC78: flash palette & periodically spawn 0x18339C / 0x183584 shot. */
static void sub_19DC78(const f3mem_t *m, uint32_t obj)
{
    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x10u) == 0u)        /* bftst 4B3A{11:1} */
        return;
    if ((rd8(m, F3_A5 + 0x0879u) & 0x03u) != 0u)        /* bftst 878{14:2} */
        return;

    uint16_t d0 = rd8(m, obj + 0x22u);
    if ((int8_t)(uint8_t)d0 >= 0x0A)
        d0 = (uint16_t)(d0 - 0x000Au);
    uint16_t rem = (uint16_t)((rd16(m, F3_A5 + 0x0878u) >> 4) % 10u);
    if (rem != d0)
        return;

    uint32_t arg = 0x00000800u | rd8(m, obj + 0x20u);
    uint32_t fn = ((rd8(m, F3_A5 + 0x4B3Bu) & 0x08u) != 0u)
                  ? 0x00183584u : 0x0018339Cu;          /* bftst 4B3A{12:1} */
    (void)Obj_Spawn(m, obj, fn, 9u, 2u, arg);
    wr8(m, obj + 0x1Du, 0x04u);
}

/* ROM 0x19D9D8: draw limb tip/base sprite at anchor + 0x20 offset. */
static void sub_19D9D8(const f3mem_t *m, uint32_t obj)
{
    uint16_t tmp = rd16(m, obj + 0x1Cu);
    wr16(m, obj + 0x1Cu, rd16(m, obj + 0x6Au));
    wr16(m, obj + 0x6Au, tmp);

    uint32_t a1 = 0x0019DF76u;
    uint16_t idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1);
    (void)Obj_AnimInit(m, obj, idx);

    uint32_t a3 = rd32(m, obj + 0x0Cu) + (uint32_t)rd16(m, obj + 0x62u) * 0x0Cu;
    uint32_t a4 = rd32(m, obj + 0x08u);
    wr16(m, a3 + 4u, (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)));
    wr16(m, a3 + 8u, (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)));
    wr16(m, a3, rd16(m, obj + 0x50u));
    wr16(m, a3 + 2u, rd16(m, obj + 0x54u));

    uint16_t dx = (uint16_t)(rd16(m, a3) - rd16(m, a3 + 4u));
    uint16_t dy = (uint16_t)(rd16(m, a3 + 2u) - rd16(m, a3 + 8u));
    uint16_t ang = Obj_Atan2(m, dx, dy);
    wr8(m, obj + 0x20u, (uint8_t)ang);

    int32_t vy = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u) * 0x20;
    int32_t vx = (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u) * 0x20;
    vx = (int32_t)((uint32_t)vx + rd32(m, a3 + 4u));
    vy = (int32_t)((uint32_t)vy + rd32(m, a3 + 8u));
    wr16(m, obj + 0x14u, (uint16_t)((uint32_t)vx >> 16));
    wr16(m, obj + 0x18u, (uint16_t)((uint32_t)vy >> 16));

    (void)anim_tick_19D876(m, obj, rd16(m, obj + 0x40u));
    center_sprite(m, obj);
    Obj_DrawSprite(m, obj);
}

/* ROM 0x19DAAA: step & draw up to 16 articulated segments, then draw head (0x19DEB4). */
static void sub_19DAAA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t d7 = 0u;
    uint16_t d6 = rd16(m, obj + 0x62u);
    uint32_t a3;
    uint16_t dx, dy;

    for (;;) {
        a3 = rd32(m, obj + 0x0Cu) + (uint32_t)d6 * 0x0Cu;
        dx = (uint16_t)(rd16(m, a3) - rd16(m, a3 + 4u));
        dy = (uint16_t)(rd16(m, a3 + 2u) - rd16(m, a3 + 8u));

        /* cmp2.w [-21, +21] at 0x19D7CE */
        if ((int16_t)dx >= -21 && (int16_t)dx <= 21
            && (int16_t)dy >= -21 && (int16_t)dy <= 21)
            break;

        uint16_t ang = Obj_Atan2(m, dx, dy);
        wr8(m, obj + 0x20u, (uint8_t)ang);
        int32_t vy = (int32_t)(int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u) * 0x38;
        int32_t vx = (int32_t)(int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u) * 0x38;
        wr32(m, a3 + 4u, rd32(m, a3 + 4u) + (uint32_t)vx);
        wr32(m, a3 + 8u, rd32(m, a3 + 8u) + (uint32_t)vy);
        wr16(m, obj + 0x14u, rd16(m, a3 + 4u));
        wr16(m, obj + 0x18u, rd16(m, a3 + 8u));

        (void)anim_tick_19D876(m, obj, rd16(m, obj + 0x40u));
        center_sprite(m, obj);
        Obj_DrawSprite(m, obj);

        d6 = (uint16_t)((d6 - 1u) & 0x000Fu);
        d7 = (uint16_t)(d7 + 1u);
        if ((int16_t)d7 > 0x000F)
            break;
    }

    if (d7 == 0u) {
        wr8(m, obj + 0x20u, rd8(m, obj + 0x68u));
        wr16(m, obj + 0x14u, rd16(m, a3));
        wr16(m, obj + 0x18u, rd16(m, a3 + 2u));
    } else if (d7 == 0x0010u) {
        d6 = (uint16_t)((d6 + 1u) & 0x000Fu);
        a3 = rd32(m, obj + 0x0Cu) + (uint32_t)d6 * 0x0Cu;
        wr16(m, obj + 0x14u, rd16(m, a3 + 4u));
        wr16(m, obj + 0x18u, rd16(m, a3 + 8u));
    } else {
        uint16_t ang = Obj_Atan2(m, dx, dy);
        wr8(m, obj + 0x20u, (uint8_t)ang);
        wr16(m, obj + 0x14u, rd16(m, a3));
        wr16(m, obj + 0x18u, rd16(m, a3 + 2u));
    }

    uint16_t tmp = rd16(m, obj + 0x1Cu);
    wr16(m, obj + 0x1Cu, rd16(m, obj + 0x6Au));
    wr16(m, obj + 0x6Au, tmp);

    uint32_t a1 = 0x0019DEB4u;
    uint16_t idx = rd16(m, a1);
    a1 += 2u;
    wr16(m, obj + 0x40u, idx);
    wr32(m, obj + 0x3Au, a1);
    (void)Obj_AnimInit(m, obj, idx);
    (void)anim_tick_19D876(m, obj, rd16(m, obj + 0x40u));
    center_sprite(m, obj);

    if (rd8(m, obj + 0x35u) != 0u) {
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + Obj_RandRange(m, 2u)));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + Obj_RandRange(m, 2u)));
    }

    if (rd8(m, obj + 0x1Du) == 0x06u)
        wr8(m, obj + 0x1Du, 0x08u);
    else if (rd8(m, obj + 0x35u) != 0u)
        wr8(m, obj + 0x1Du, 0x06u);
    else
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));

    wr8(m, obj + 0x35u, 0u);
    draw_sprite_regs(m, r);

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {
        uint32_t p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0906u, p + 4u);
        p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        p += 4u;
        wr32(m, F3_A5 + 0x08FEu, p);
        r->a[1] = p;
    }
}

int Fn_19D876(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x02u) != 0u)        /* 0x19D876 bftst 4B3A{14:1} */
        wr32(m, obj + 0x24u, 0u);

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x19D884 -> 0x19E540 */
        wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u));
        wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u));

        wr16(m, F3_A5 + 0x4BC2u, (uint16_t)(rd16(m, F3_A5 + 0x4BC2u) + 1u));
        int32_t recv = (int32_t)sub_17B02E(m, obj);
        if (recv >= 0) {
            uint32_t a1 = (uint32_t)recv;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + 0x00007530u);
        }
        if (rd8(m, obj + 0x35u) != 0u)
            wr32(m, F3_A5 + 0x0886u, rd32(m, F3_A5 + 0x0886u) + 0x00012C5Fu);

        wr8(m, F3_A5 + 0x4B26u, rd8(m, F3_A5 + 0x4B26u) | 0x20u); /* {2:1} */
        wr16(m, F3_A5 + 0x4B20u, rd16(m, obj + 0x34u));
        wr32(m, F3_A5 + 0x4B1Cu, rd32(m, obj + 0x36u));
        wr8(m, obj + 0x23u, rd8(m, obj + 0x23u) | 0x04u);
        wr16(m, F3_A5 + 0x4B22u, (uint16_t)(rd16(m, F3_A5 + 0x4B22u) + 1u));
        wr8(m, obj + 0x1Du, 0u);
        (void)Obj_Spawn(m, obj, 0x00179F6Cu, 5u, 2u, 0u);

        wr32(m, obj + 0x44u, 0x0019E65Eu);
        wr16(m, obj + 0x48u, rd16(m, 0x0019E662u));
        wr16(m, obj + 0x4Au, 0u);
        wr16(m, obj + 0x42u, Obj_RandRange(m, 0x0080u));

        if ((int16_t)rd16(m, obj + 0x54u) > 0x012C
            || (int16_t)rd16(m, F3_A5 + 0x4B38u) == -1) {
            wr8(m, obj + 0x23u, rd8(m, obj + 0x23u) | 0x04u);
            wr16(m, obj + 0x06u, 1u);
            wr32(m, obj, 0x0017AADAu);
            return 0;
        }
        (void)script10_step_19D876(m, obj);
        wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
        wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
        (void)anim_tick_19D876(m, obj, rd16(m, obj + 0x40u));
        center_sprite(m, obj);
        draw_sprite_regs(m, r);
        if ((rd8(m, F3_A5 + 0x1D58u) & 0x01u) == 0u
            && (rd8(m, F3_A5 + 0x0879u) & 0x03u) == 0u) {
            r->d[0] = Obj_Spawn(m, obj, 0x00179F26u, 5u, 2u, 0u);
        }
        wr32(m, obj, 0x0019E65Au);
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x20u) == 0u) {      /* 0x19D8A8 bftst 4B3A{10:1} */
        sub_19DD78(m, obj);
        wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
        wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
        (void)anim_tick_19D876(m, obj, rd16(m, obj + 0x40u));
        center_sprite(m, obj);
        if ((rd8(m, F3_A5 + 0x4B3Bu) & 0x01u) == 0u) {
            uint32_t p = rd32(m, F3_A5 + 0x08FEu);
            wr32(m, p, obj);
            wr32(m, F3_A5 + 0x08FEu, p + 4u);
            p = rd32(m, F3_A5 + 0x0906u);
            wr32(m, p, obj);
            wr32(m, F3_A5 + 0x0906u, p + 4u);
        }
        if (rd8(m, obj + 0x35u) != 0u) {
            wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + Obj_RandRange(m, 2u)));
            wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + Obj_RandRange(m, 2u)));
        }
        wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
        wr8(m, obj + 0x35u, 0u);
        sub_19DC78(m, obj);
        draw_sprite_regs(m, r);
        wr8(m, obj + 0x34u, 0x0Au);
        wr32(m, obj, 0x0019D876u);
        return 0;
    }

    if (rd32(m, obj + 0x44u) == 0x0019DD3Cu)            /* 0x19D942 */
        sub_19DD5A(m, obj);
    (void)script10_step_19D876(m, obj);
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));
    uint16_t saved_20 = rd16(m, obj + 0x20u);
    sub_19D9D8(m, obj);
    sub_19DAAA(m, r);
    wr16(m, obj + 0x20u, saved_20);
    wr8(m, obj + 0x34u, 0x14u);
    wr16(m, obj + 0x62u, (uint16_t)((rd16(m, obj + 0x62u) + 1u) & 0x000Fu));
    wr32(m, obj, 0x0019D876u);
    return 0;
}
