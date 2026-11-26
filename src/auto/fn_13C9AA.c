/* ROM 0x13C9AA: enemy tick (bra.w 0x13C85E, A0 = object).
 * Handles boss-clear (A5-0x144B bit 4) -> spawn 0x13E4EE + sound 0x16 + arm
 * 0x13C9F8; death (+0x24 <= 0) -> relay score +0x4C to 0x0CBDDE then same;
 * bomb/explode (A5+0x1D58 bit 1 & X in [-10..0x11C]) -> 0x0CBB2C. Normal
 * tick integrates (+0x50,+0x52) via 0x13CA14, adds +0x42 to +0x52, draws
 * body sprite 0x3C5B0 and animated tail sprite 0x13CA74[+0x3A] at X+0x50,
 * fires 3x 0x0CCD04 every 0x14 frames when +0x3E > 0xC0, pushes to 0x8FE/
 * 0x906, frees via 0x0105EC if X <= -0x32, else re-arms 0x13C9AA. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x0CC2BC: frame-rate-compensated modulo timer -> D0w = rem, D0hi = quot. */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)d0w * (uint32_t)d1w) << 8;
    d1 = (d1 >> 16) | (d1 << 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t d0 = rd16(m, F3_A5 + 0x0878u);
    uint16_t quot = (uint16_t)(d0 / (uint16_t)d1);
    uint16_t rem = (uint16_t)(d0 % (uint16_t)d1);
    *d0_io = ((uint32_t)quot << 16) | rem;
    *d1_io = d1;
}

int Fn_13C9AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x13C85E */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x30100005u); /* 0x13C9BE */
        Sound_PlayHooked(m, 0x16u);                     /* 0x13C9DC */
        wr32(m, obj, 0x0013C9F8u);                      /* 0x13C9F0 */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x13C868 */
        uint32_t a1 = Obj_0CBDDE(m, obj);               /* 0x13C9AE */
        uint32_t d0 = rd16(m, obj + 0x4Cu);             /* 0x13C9B4..0x13C9B6 */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);  /* 0x13C9BA */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x30100005u); /* 0x13C9BE */
        Sound_PlayHooked(m, 0x16u);                     /* 0x13C9DC */
        wr32(m, obj, 0x0013C9F8u);                      /* 0x13C9F0 */
        r->a[1] = a1;
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x13C870 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {              /* 0x13C878..0x13C886 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x13C888 */
            wr32(m, obj + 0x10u, 0x0003C664u);          /* 0x13CA06 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);     /* 0x0CBD12 */
            uint32_t d0 = rd16(m, obj + 0x4Cu);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            r->d[0] = d0;
            r->a[1] = a1;
            explode_tick_regs(m, r);                    /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x13C894 -> 0x13CA14: integrate velocity (+0x50, +0x52) << 6 into (+0x14, +0x18). */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u)
         + (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x50u) << 6));
    uint32_t vy = (uint32_t)((int32_t)(int16_t)rd16(m, obj + 0x52u) << 6);
    if ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u)
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
    else
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - vy);

    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + rd16(m, obj + 0x42u))); /* 0x13C898 */
    Obj_11C32C(m, obj);                                 /* 0x13C8A0 */
    wr32(m, obj + 0x10u, 0x0003C5B0u);                  /* 0x13C8A6 */
    draw_sprite_regs(m, r);                             /* 0x13C8AE */

    uint16_t sx = rd16(m, obj + 0x14u);                 /* 0x13C8B4 */
    wr16(m, obj + 0x14u, (uint16_t)(sx + 0x0050u));     /* 0x13C8B8 */
    uint8_t idx = rd8(m, obj + 0x3Au);                  /* 0x13C8C2 */
    wr32(m, obj + 0x10u, rd32(m, 0x0013CA74u + (uint32_t)idx * 4u)); /* 0x13C8BE..0x13C8C8 */
    draw_sprite_regs(m, r);                             /* 0x13C8CE */
    wr16(m, obj + 0x14u, sx);                           /* 0x13C8D4 */

    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    uint16_t w3e = (uint16_t)(rd16(m, obj + 0x3Eu) + 4u); /* 0x13C8D8 */
    wr16(m, obj + 0x3Eu, w3e);
    if ((int16_t)w3e > 0x0100) {                        /* 0x13C8DC */
        w3e = 0x0100u;
        wr16(m, obj + 0x3Eu, w3e);                      /* 0x13C8E6 */
    }

    d0 = setw(d0, rd16(m, obj + 0x3Eu));                /* 0x13C8EC */
    wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + (uint16_t)d0)); /* 0x13C8F0 */
    if ((int8_t)rd8(m, obj + 0x3Au) >= 6)               /* 0x13C8F4 */
        wr8(m, obj + 0x3Au, 0u);                        /* 0x13C8FE */

    if ((int16_t)rd16(m, obj + 0x3Eu) > 0x00C0) {       /* 0x13C902 */
        sub_0CC2BC(m, 0x0014u, &d0, &d1);               /* 0x13C90C..0x13C910 */
        if ((uint16_t)d0 == 0u) {                       /* 0x13C916 */
            d1 = ((rd8(m, obj + 0x1Cu) & 0x02u) == 0u) ? 0x0E04C000u : 0x0E1B4000u; /* 0x13C91C..0x13C930 */
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);
            d1 = setw(d1, (uint16_t)((uint16_t)d1 + 0x2000u));
            (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);
            d1 = setw(d1, (uint16_t)((uint16_t)d1 - 0x4000u));
            d0 = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);
        }
    }

    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) - 0x0020u)); /* 0x13C98C */

    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);             /* 0x13C992 -> 0x0CBDA4 */
    wr32(m, a1, obj);
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);
    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);

    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0032)       /* 0x13C998 */
        Obj_Free(m, obj);                               /* 0x13C9F8 */
    else
        wr32(m, obj, 0x0013C9AAu);                      /* 0x13C9A2 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
