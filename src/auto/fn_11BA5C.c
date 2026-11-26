/* ROM 0x11BA5C: enemy projectile/debris burst tick (A0 = object).
 * Handles bomb-clear (A5+0x1D58 bit 1 and X in [-10, 0x11C] -> 0x0CBD12
 * score + 0x0CBB2C explode). Otherwise sets speed +0x42 = (+0x06 >> 1),
 * moves along angle +0x20 (0x0CBE12), draws sprite (0x001282), frees self
 * (0x0105EC) if X leaves [-32, 320] or Y leaves [-32, 232] (0x0CC112/0x0CC116
 * cmp2.w bounds), and counts down +0x06, re-arming 0x11BA5C while > 0 or
 * clearing (+0x1E, +0x3E, +0x3A), offsetting (+0x14, +0x18) by (-4, -4), and
 * transitioning to 0x11BAD6. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../player/player_move.h"
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

int Fn_11BA5C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x11BA5C */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {              /* 0x11BA64..0x11BA72 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);     /* 0x11BA74 -> 0x0CBD12 */
            uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            r->a[1] = a1;
            r->d[0] = d0;
            explode_tick_regs(m, r);                    /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    {
        uint16_t spd = (uint16_t)(rd16(m, obj + 0x06u) >> 1);
        uint16_t ang = rd16(m, obj + 0x20u);
        int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + (uint32_t)ang * 2u);
        int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + (uint32_t)ang * 2u);
        r->d[1] = (uint32_t)((int32_t)(int16_t)spd * (int32_t)cos_v);
        r->d[2] = (uint32_t)((int32_t)(int16_t)spd * (int32_t)sin_v);
        r->a[3] = 0x00027E40u;
        wr16(m, obj + 0x42u, spd);                      /* 0x11BA7A..0x11BA80 */
        Obj_MoveByAngle(m, obj, ang);                   /* 0x11BA84..0x11BA88 */
    }
    draw_sprite_regs(m, r);                             /* 0x11BA8E */

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);      /* 0x11BA94 */
        r->d[0] = setw(r->d[0], (uint16_t)x);
        if (x < (int16_t)rd16(m, 0x000CC112u)
            || x > (int16_t)rd16(m, 0x000CC114u)) {     /* 0x11BA98 cmp2.w */
            Obj_Free(m, obj);                           /* 0x11BC1A */
            return 0;
        }
    }

    {
        int16_t y = (int16_t)rd16(m, obj + 0x18u);      /* 0x11BAA4 */
        r->d[0] = setw(r->d[0], (uint16_t)y);
        if (y < (int16_t)rd16(m, 0x000CC116u)
            || y > (int16_t)rd16(m, 0x000CC118u)) {     /* 0x11BAA8 cmp2.w */
            Obj_Free(m, obj);                           /* 0x11BC1A */
            return 0;
        }
    }

    {
        int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x11BAB4 */
        wr16(m, obj + 0x06u, (uint16_t)cnt);
        if (cnt > 0) {                                  /* 0x11BAB8 */
            wr32(m, obj, 0x0011BA5Cu);                  /* 0x11BA54 */
            return 0;
        }
    }

    wr16(m, obj + 0x1Eu, 0u);                           /* 0x11BABA */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 4u)); /* 0x11BABE */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 4u)); /* 0x11BAC2 */
    wr16(m, obj + 0x3Eu, 0u);                           /* 0x11BAC6 */
    wr16(m, obj + 0x3Au, 0u);                           /* 0x11BACA */
    wr32(m, obj, 0x0011BAD6u);                          /* 0x11BACE */
    return 0;
}
