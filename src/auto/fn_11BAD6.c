/* ROM 0x11BAD6: debris/transform anim tick (A0 = object).
 * Steps anim table 0x11BC28 via 0x0CBD24. When the table hits -1, adjusts
 * pos by (-8,-8), installs hitbox 7..0x17 x 8..0x18, clears +0x06 and
 * transitions to 0x11BB3C. Otherwise handles bomb-clear (0x0CBD12 -> 0x0CBB2C)
 * when A5+0x1D58 bit 1 is set and X in [-10, 0x11C], else draws sprite
 * (0x001282) and re-arms 0x11BAD6. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
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
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    r->a[1] = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        r->d[0] = d0;
        r->d[1] = setw(r->d[1], d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x00649A with full D0 result. */
static uint32_t dist_full(const f3mem_t *m, uint16_t dx_in, uint16_t dy_in)
{
    uint32_t d0 = (uint32_t)((int32_t)(int16_t)dx_in * (int32_t)(int16_t)dx_in);
    uint32_t d1 = (uint32_t)((int32_t)(int16_t)dy_in * (int32_t)(int16_t)dy_in);
    d0 += d1;
    d0 += d0;

    uint16_t shift = 4u;
    for (int i = 0; i < 3; i++) {
        if ((int32_t)d0 < 0x200)
            break;
        d0 >>= 8;
        shift = (uint16_t)(shift + 4u);
    }

    int32_t idx = (int32_t)(int16_t)(uint16_t)d0;
    d0 = (d0 & 0xFFFF0000u) | (uint32_t)rd16(m, 0x028340u + (uint32_t)(idx * 2));
    d0 <<= (shift & 63u);
    return (d0 << 16) | (d0 >> 16);
}

/* ROM 0x0CBB2C with register tracking. */
static void explode_tick_regs(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t dx = (uint16_t)(rd16(m, obj + 0x14u) - 0x00A0u);
    uint16_t dy = (uint16_t)(rd16(m, obj + 0x18u) - 0x0074u);

    uint32_t a1 = rd32(m, obj + 0x10u);
    int16_t off_y = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Eu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1)) >> 5;
    int16_t off_x = (int16_t)((int32_t)(int16_t)((uint16_t)rd8(m, obj + 0x1Fu) - 0x0100u)
                              * (int32_t)(int16_t)rd16(m, a1 + 2u)) >> 5;
    dx = (uint16_t)(dx - (uint16_t)off_x);
    dy = (uint16_t)(dy - (uint16_t)off_y);

    uint32_t d0_dist = dist_full(m, dx, dy);
    uint16_t d0w = (uint16_t)((uint16_t)d0_dist << 2);
    uint16_t thresh = rd16(m, F3_A5 + 0x1D5Cu);
    int is_lt = (int16_t)d0w < (int16_t)thresh;
    uint16_t diff = (uint16_t)(d0w - thresh);

    Obj_ExplodeTick(m, obj);

    if (is_lt) {
        r->a[1] = a1 + 4u;
        r->d[0] = setw(d0_dist, diff);
        r->d[1] = setw(r->d[1], dy);
        return;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        uint16_t ang = Obj_Atan2(m, dx, dy);
        r->a[1] = a1 + 4u;
        r->d[0] = setw(d0_dist, (uint16_t)(ang << 8));
        r->d[1] = setw(r->d[1], dy);
        return;
    }

    draw_sprite_regs(m, r);
}

/* ROM 0x0CBD24 with D0/D1 register tracking. */
static void sub_0CBD24(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0_io = setw(*d0_io, off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur <= 0)
            return;
        uint16_t cnt = (uint16_t)(rd16(m, obj + 0x3Eu) + 1u);
        wr16(m, obj + 0x3Eu, cnt);
        *d1_io = setw(*d1_io, cnt);
        if ((int16_t)cnt <= dur) {
            wr32(m, obj + 0x10u, rd32(m, tbl + (uint32_t)(int32_t)(int16_t)off + 2u));
            return;
        }
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x3Au, (uint16_t)(off + 6u));
    }
}

int Fn_11BAD6(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t tbl = 0x0011BC28u;                           /* 0x11BAD6 */

    sub_0CBD24(m, obj, tbl, &d0, &d1);                    /* 0x11BADC */
    r->d[0] = d0;
    r->d[1] = d1;

    if ((int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)(uint16_t)d0) < 0) { /* 0x11BAE2..0x11BAE6 */
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u)); /* 0x11BB10 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u)); /* 0x11BB14 */
        wr16(m, obj + 0x2Cu, 0x0007u);                    /* 0x11BB18 */
        wr16(m, obj + 0x30u, 0x0008u);                    /* 0x11BB1E */
        wr16(m, obj + 0x2Eu, 0x0017u);                    /* 0x11BB24 */
        wr16(m, obj + 0x32u, 0x0018u);                    /* 0x11BB2A */
        wr16(m, obj + 0x06u, 0u);                         /* 0x11BB30 */
        wr32(m, obj, 0x0011BB3Cu);                        /* 0x11BB34 */
        r->a[1] = tbl;
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {        /* 0x11BAEA */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {                /* 0x11BAF2..0x11BB00 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);       /* 0x11BB02 -> 0x0CBD12 */
            uint32_t add = (uint32_t)rd16(m, obj + 0x4Cu);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + add);
            r->a[1] = a1;
            r->d[0] = add;
            explode_tick_regs(m, r);
            return 0;
        }
    }

    draw_sprite_regs(m, r);                               /* 0x11BB08 */
    wr32(m, obj, 0x0011BAD6u);                            /* 0x11BB0E -> 0x11BACE */
    return 0;
}
