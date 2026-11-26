/* ROM 0x13A6A2: spinning/tumbling boss debris or enemy tick (A0 = object).
 * Checks bomb-clear (A5-0x144B bit 4) and HP (+0x24 <= 0 -> score via 0x0CBDDE,
 * spawn 0x13E4EE explosion, Sound_PlayHooked 0x35, transition to 0x13A796).
 * Otherwise updates sprite from 0x13A7A4[+0x3A byte], advances +0x3A by +0x3E
 * (masked to 0x7FF), decays +0x3E down to >= 0x40, checks on-screen bomb/explode
 * (0x0CBD12 -> 0x0CBB2C), integrates (+0x44, +0x48) into (+0x14, +0x18) with
 * -0x2000 X accel, updates flash (0x11C32C), draws sprite (0x001282), appends to
 * collision lists (0x0CBDA4), and frees via 0x0105EC when X <= -0x20 else
 * re-arms 0x13A748. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
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

/* ROM 0x0CBDA4: append obj to both A5+0x8FE and A5+0x906. */
static void sub_0CBDA4(const f3mem_t *m, uint32_t obj, uint32_t *a1_out)
{
    uint32_t a1 = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x08FEu, a1);

    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_out = a1;
}

/* ROM 0x00649A with full D0 result (high word = shifted<<16 | ...). */
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

/* ROM 0x0CBB2C with register tracking for tail-jump from 0x0CBD12. */
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

int Fn_13A6A2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)        /* 0x13A6A2..0x13A6A8 */
        goto die_explode;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x13A6AC..0x13A6B0 */
        uint32_t a1 = Obj_0CBDDE(m, obj);               /* 0x13A74C */
        r->a[1] = a1;
        uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);  /* 0x13A752..0x13A754 */
        r->d[0] = pts;
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + pts); /* 0x13A758 */
        goto die_explode;
    }

    {
        uint32_t idx = rd8(m, obj + 0x3Au);             /* 0x13A6B4 */
        wr32(m, obj + 0x10u, rd32(m, 0x0013A7A4u + idx * 4u)); /* 0x13A6BA */
    }

    {
        uint16_t step = rd16(m, obj + 0x3Eu);           /* 0x13A6C4 */
        r->d[0] = setw(r->d[0], step);
        wr16(m, obj + 0x3Au, (uint16_t)((rd16(m, obj + 0x3Au) + step) & 0x07FFu)); /* 0x13A6C8..0x13A6CC */
        uint16_t spd = (uint16_t)(rd16(m, obj + 0x3Eu) - 0x0010u); /* 0x13A6D2 */
        if ((int16_t)spd < 0x0040)
            spd = 0x0040u;                              /* 0x13A6E2 */
        wr16(m, obj + 0x3Eu, spd);
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x13A6E8 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {              /* 0x13A6F0..0x13A6FE */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x13A700 */
            /* 0x13A706 -> 0x0CBD12 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
            r->a[1] = a1;
            uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);
            r->d[0] = pts;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + pts);
            explode_tick_regs(m, r);                    /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    /* 0x13A70C */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + rd32(m, obj + 0x44u));
    {
        uint32_t vy = rd32(m, obj + 0x48u);
        r->d[0] = vy;
        wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);
    }
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) - 0x00002000u);

    Obj_11C32C(m, obj);                                 /* 0x13A724 */
    draw_sprite_regs(m, r);                             /* 0x13A72A */
    sub_0CBDA4(m, obj, &r->a[1]);                       /* 0x13A730 */

    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0020) {     /* 0x13A736 */
        Obj_Free(m, obj);                               /* 0x13A796 */
        return 0;
    }

    wr32(m, obj + 0x00u, 0x0013A748u);                  /* 0x13A740 */
    return 0;

die_explode:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x10100002u); /* 0x13A75C..0x13A776 */
    Sound_PlayHooked(m, 0x0035u);                       /* 0x13A77A..0x13A78A */
    wr32(m, obj + 0x00u, 0x0013A796u);                  /* 0x13A78E */
    return 0;
}
