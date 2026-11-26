/* ROM 0x1939EC: boss sub-part/head tick (A0 = object).
 * If A5+0x4B38 == -1, spawns 0x179D92 and frees itself. Otherwise snaps
 * (+0x14, +0x18) to (+0x50, +0x54), ticks 0x179A5C animation, centers by
 * sprite dimensions * 8. If +0x22 != 0x0A only draws (if A5+0x4B50 != 0) and
 * re-arms 0x1939EC. If +0x22 == 0x0A, switches to anim 0x194D28 when A5+0x4B3A
 * bit 9 (from MSB) is set and A5+0x4B50 == 0x80, runs 0x17B9BA+0x0CBB0C or
 * 0x17BA02 hit-spark checks, relays damage via 0x17B92C, pushes collision
 * lists if A5+0x4B3A bit 15 clear, draws sprite, optionally spawns 0x18098C,
 * and re-arms 0x1939EC. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
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

/* ROM 0x0CBB0C: set +0x35 = 1, +0x36 = A5+0x1D54, +0x24 -= 0x4000 when A5+0x1D58 bit 2 set. */
static void sub_0CBB0C(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x04u) != 0u) {
        wr8(m, obj + 0x35u, 1u);
        wr32(m, obj + 0x36u, rd32(m, F3_A5 + 0x1D54u));
        wr32(m, obj + 0x24u, rd32(m, obj + 0x24u) - 0x00004000u);
    }
}

/* ROM 0x17BA76: compute hit-spark point (out_x, out_y); returns < 0 if none. */
static int sub_17BA76(const f3mem_t *m, uint32_t obj, int16_t *out_x, int16_t *out_y)
{
    uint32_t a1 = rd32(m, obj + 0x36u);
    if (rd8(m, obj + 0x35u) == 2u) {
        if ((uint16_t)(rd16(m, F3_A5 + 0x0878u) % 3u) != 0u)
            return -1;
        int16_t hw = (int16_t)((int16_t)(rd16(m, obj + 0x2Eu) - rd16(m, obj + 0x2Cu)) >> 1);
        int16_t hh = (int16_t)((int16_t)(rd16(m, obj + 0x32u) - rd16(m, obj + 0x30u)) >> 1);
        int16_t ry = (int16_t)Obj_RandRange(m, (uint16_t)hh);
        int16_t rx = (int16_t)Obj_RandRange(m, (uint16_t)hw);
        *out_x = (int16_t)(rx + (int16_t)rd16(m, obj + 0x14u) + (int16_t)rd16(m, obj + 0x2Cu) + hw);
        *out_y = (int16_t)(ry + (int16_t)rd16(m, obj + 0x18u) + (int16_t)rd16(m, obj + 0x30u) + hh);
        return 0;
    }

    uint8_t st = rd8(m, a1 + 0x22u);
    if (st != 0u && st != 4u && (rd16(m, F3_A5 + 0x0878u) & 3u) != 0u)
        return -1;

    int16_t x0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Cu));
    int16_t y0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x30u));
    int16_t x1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Cu));
    int16_t y1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x30u));
    int16_t minx = (x1 > x0) ? x1 : x0;
    int16_t miny = (y1 > y0) ? y1 : y0;

    int16_t rx0 = (int16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x2Eu));
    int16_t ry0 = (int16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x32u));
    int16_t rx1 = (int16_t)(rd16(m, a1 + 0x14u) + rd16(m, a1 + 0x2Eu));
    int16_t ry1 = (int16_t)(rd16(m, a1 + 0x18u) + rd16(m, a1 + 0x32u));
    int16_t maxx = (rx1 < rx0) ? rx1 : rx0;
    int16_t maxy = (ry1 < ry0) ? ry1 : ry0;

    *out_x = (int16_t)(minx + (int16_t)((uint16_t)(maxx - minx) >> 1));
    *out_y = (int16_t)(miny + (int16_t)((uint16_t)(maxy - miny) >> 1));
    return 0;
}

/* ROM 0x17B9BA: spawn hit-spark 0x17A122 if +0x35 != 0 and Obj_RandRange(5) == 0. */
static void sub_17B9BA(const f3mem_t *m, uint32_t obj)
{
    int16_t hx, hy;
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (Obj_RandRange(m, 5u) != 0u)
        return;
    if (sub_17BA76(m, obj, &hx, &hy) < 0)
        return;
    uint16_t dx = (uint16_t)(hx - (int16_t)rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(hy - (int16_t)rd16(m, obj + 0x54u));
    uint32_t arg = ((uint32_t)dx << 16) | dy;
    (void)Obj_Spawn(m, obj, 0x0017A122u, 5u, 2u, arg);
}

/* ROM 0x17BA02: spawn hit-spark 0x17A016 if +0x35 != 0. */
static void sub_17BA02(const f3mem_t *m, uint32_t obj)
{
    int16_t hx, hy;
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    if (sub_17BA76(m, obj, &hx, &hy) < 0)
        return;
    uint16_t dx = (uint16_t)(hx - (int16_t)rd16(m, obj + 0x50u));
    uint16_t dy = (uint16_t)(hy - (int16_t)rd16(m, obj + 0x54u));
    uint32_t arg = ((uint32_t)dx << 16) | dy;
    (void)Obj_Spawn(m, obj, 0x0017A016u, 5u, 2u, arg);
}

/* ROM 0x17B92C: relay damage (+0x24) to parent (+0x08) scaled by phase multiplier. */
static void sub_17B92C(const f3mem_t *m, uint32_t obj)
{
    if (rd8(m, obj + 0x35u) == 0u)
        return;
    int32_t dmg = (int32_t)rd32(m, obj + 0x24u);
    if (rd8(m, obj + 0x35u) == 2u) {
        dmg = (dmg / 100) * (int32_t)rd16(m, F3_A5 + 0x4B2Eu);
    } else {
        uint32_t a1 = rd32(m, obj + 0x36u);
        uint8_t st = rd8(m, a1 + 0x22u);
        if (st == 1u)
            dmg = (dmg / 100) * (int32_t)rd16(m, F3_A5 + 0x4B2Au);
        else if (st == 2u)
            dmg = (dmg / 100) * (int32_t)rd16(m, F3_A5 + 0x4B2Cu);
    }
    uint32_t parent = rd32(m, obj + 0x08u);
    wr32(m, parent + 0x24u, rd32(m, parent + 0x24u) + (uint32_t)dmg);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, parent + 0x36u, rd32(m, obj + 0x36u));
    wr16(m, parent + 0x34u, rd16(m, obj + 0x34u));
}

int Fn_1939EC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (rd16(m, F3_A5 + 0x4B38u) == 0xFFFFu) {           /* 0x1939EC */
        r->d[0] = Obj_Spawn(m, obj, 0x00179D92u, 5u, 2u, 0u);
        Obj_Free(m, obj);
        return 0;
    }

    /* Check animation index before mutating state so unsupported indices fail cleanly. */
    uint16_t anim_idx = rd16(m, obj + 0x40u);
    if (anim_idx == 3u || anim_idx == 11u || anim_idx == 13u || anim_idx > 16u)
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));          /* 0x1939F6 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));          /* 0x1939FC */
    if (Obj_AnimTick(m, obj, anim_idx) < 0)              /* 0x193A02..0x193A10 */
        return -1;

    /* 0x193A12..0x193A22: center sprite by {h, w} * 8 */
    {
        uint32_t spr = rd32(m, obj + 0x10u);
        uint16_t d1w = (uint16_t)(rd16(m, spr) << 3);
        uint16_t d0w = (uint16_t)(rd16(m, spr + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - d0w));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - d1w));
        r->a[1] = spr + 4u;
        r->d[0] = setw(r->d[0], d0w);
        r->d[1] = setw(r->d[1], d1w);
    }

    if (rd8(m, obj + 0x22u) != 0x0Au) {                  /* 0x193A26 */
        if (rd16(m, F3_A5 + 0x4B50u) != 0u) {
            wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));
            draw_sprite_regs(m, r);
        }
        wr32(m, obj, 0x001939ECu);                       /* 0x1939E4 */
        return 0;
    }

    /* 0x193A44..0x193A72: switch animation to 0x194D28 when whip hits 0x80 */
    if ((rd16(m, F3_A5 + 0x4B3Au) & 0x0040u) != 0u
        && rd16(m, F3_A5 + 0x4B50u) == 0x0080u) {
        uint32_t a1 = 0x00194D28u;
        uint16_t idx = rd16(m, a1);
        a1 += 2u;
        wr16(m, obj + 0x40u, idx);
        wr32(m, obj + 0x3Au, a1);
        (void)Obj_AnimInit(m, obj, idx);
    }

    if (rd32(m, obj + 0x10u) == 0x0003F770u) {           /* 0x193A74 */
        sub_17B9BA(m, obj);
        wr8(m, obj + 0x34u, 0x0Au);
        wr8(m, obj + 0x35u, 0u);
        sub_0CBB0C(m, obj);
    } else {
        sub_17BA02(m, obj);
    }

    sub_17B92C(m, obj);                                  /* 0x193AA0 */
    wr8(m, obj + 0x34u, 0x10u);                          /* 0x193AA6 */

    if ((rd16(m, F3_A5 + 0x4B3Au) & 0x0001u) == 0u) {    /* 0x193AAC bftst {15:1} */
        uint32_t p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0906u, p + 4u);
        p = rd32(m, F3_A5 + 0x08FEu);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x08FEu, p + 4u);
    }

    wr8(m, obj + 0x1Du, rd8(m, F3_A5 + 0x4B36u));        /* 0x193ACA */
    wr8(m, obj + 0x35u, 0u);                             /* 0x193AD0 */
    draw_sprite_regs(m, r);                              /* 0x193AD4 */

    if (rd16(m, F3_A5 + 0x4B38u) != 0u) {                /* 0x193ADA */
        uint32_t d0 = rd32(m, F3_A5 + 0x4B10u);          /* 0x193AE4 */
        r->d[0] = d0;
        if ((int32_t)d0 > (int32_t)rd32(m, F3_A5 + 0x4B0Cu)
            && rd8(m, obj + 0x1Du) != 0u) {
            r->d[0] = Obj_Spawn(m, obj, 0x0018098Cu, 9u, 2u, 0x0AFA0000u);
        }
    }

    wr32(m, obj, 0x001939ECu);                           /* 0x1939E4 */
    return 0;
}
