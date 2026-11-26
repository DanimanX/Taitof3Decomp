/* ROM 0x13BA06: diving/swooping enemy tick (A0 = object).
 * While inside [-0x10..0x110] x [8..0xC0] and (+0x22 -= 0x60) > 0, runs dive
 * phase (0x13B926): animates via 0x13BC34, checks bomb/explode (0x0CBD12),
 * moves by +0x22.b speed along +0x20 angle (0x0CBE12), draws (0x001282),
 * appends to lists (0x0CBDA4), and spawns 3 0x0CCD5C projectiles if +0x3C < 3.
 * Otherwise decrements +0x06; if > 0 aims at player (A5+0x1D28) via 0x0CC3C6,
 * resets +0x42=4/+0x22=0x78, animates/moves/draws, and transitions to 0x13BABE.
 * When +0x06 <= 0, clears +0x42/+0x22/+0x3A/+0x3E and runs exit/accelerate phase
 * (0x13BADE): plays 0x13BC4E anim, switches to 0x13BC6E[+0x3A>>4] spin, ramps
 * +0x3E up to 0x100, re-aims once at player, moves/draws, and frees via
 * 0x0105EC outside [-0x40..0x180] x [-0x40..0x128] else transitions to 0x13BBE0. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../enemy/obj_E8B72.h"
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
    wr32(m, F3_A5 + 0x08FEu, a1 + 4u);

    a1 = rd32(m, F3_A5 + 0x0906u);
    wr32(m, a1, obj);
    a1 += 4u;
    wr32(m, F3_A5 + 0x0906u, a1);
    *a1_out = a1;
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

/* ROM 0x0CBD52 with D0/D1 register tracking. */
static void sub_0CBD52(const f3mem_t *m, uint32_t obj, uint32_t tbl,
                       uint32_t *d0_io, uint32_t *d1_io)
{
    for (;;) {
        uint16_t off = rd16(m, obj + 0x3Au);
        *d0_io = setw(*d0_io, off);
        int16_t dur = (int16_t)rd16(m, tbl + (uint32_t)(int32_t)(int16_t)off);
        if (dur < 0) {
            wr16(m, obj + 0x3Au, 0u);
            continue;
        }
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

/* ROM 0x0CBE12 with D0/D1/A1 register tracking. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t ang)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t cos_v = (int16_t)rd16(m, 0x00027E40u + 0x80u + 2u * ang);
    int16_t sin_v = (int16_t)rd16(m, 0x00027E40u + 2u * ang);
    int32_t vx = (int32_t)speed * (int32_t)cos_v;
    int32_t vy = (int32_t)speed * (int32_t)sin_v;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)vy);

    r->a[1] = 0x00027E40u;
    r->d[0] = setw(r->d[0], ang);
    r->d[1] = (uint32_t)vx;
}

/* ROM 0x0CC3C6 with D0/D1 register tracking. */
static uint16_t sub_0CC3C6(const f3mem_t *m, uint32_t obj, uint32_t target,
                           uint32_t *d0_io, uint32_t *d1_io)
{
    int16_t cx = (int16_t)((int16_t)(rd16(m, target + 0x2Cu) + rd16(m, target + 0x2Eu)
                                   - rd16(m, obj + 0x2Cu) - rd16(m, obj + 0x2Eu)) >> 1);
    uint16_t dx = (uint16_t)(cx + rd16(m, target + 0x14u) - rd16(m, obj + 0x14u));

    int16_t cy = (int16_t)((int16_t)(rd16(m, target + 0x30u) + rd16(m, target + 0x32u)
                                   - rd16(m, obj + 0x30u) - rd16(m, obj + 0x32u)) >> 1);
    uint16_t dy = (uint16_t)(cy + rd16(m, target + 0x18u) - rd16(m, obj + 0x18u));

    uint16_t ang = Obj_Atan2(m, dx, dy);
    *d0_io = setw(*d0_io, ang);
    *d1_io = setw(*d1_io, dy);
    return ang;
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

/* ROM 0x0CBB2C with register tracking for 0x0CBD12 tail jump. */
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

static void relay_explode(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
    uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);
    r->a[1] = a1;
    uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);
    r->d[0] = pts;
    wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + pts);
    explode_tick_regs(m, r);
}

static int on_screen_bomb(const f3mem_t *m, uint32_t obj)
{
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u)
        return 0;
    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    return (x >= -0x000A && x <= 0x011C);
}

int Fn_13BA06(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if (x > -0x0010 && x < 0x0110 && y > 0x0008 && y < 0x00C0) {
            uint16_t v22 = (uint16_t)(rd16(m, obj + 0x22u) - 0x0060u);
            wr16(m, obj + 0x22u, v22);
            if ((int16_t)v22 > 0) {                     /* 0x13BA34 -> 0x13B926 */
                if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
                    goto die_explode;
                if ((int32_t)rd32(m, obj + 0x24u) <= 0)
                    goto die_score;

                r->a[1] = 0x0013BC34u;
                sub_0CBD52(m, obj, 0x0013BC34u, &r->d[0], &r->d[1]);

                if (on_screen_bomb(m, obj)) {
                    relay_explode(m, r);
                    return 0;
                }

                int16_t spd = (int16_t)(int8_t)rd8(m, obj + 0x22u);
                wr16(m, obj + 0x42u, (uint16_t)spd);
                move_by_angle_regs(m, r, rd16(m, obj + 0x20u));
                Obj_11C32C(m, obj);
                draw_sprite_regs(m, r);
                sub_0CBDA4(m, obj, &r->a[1]);

                if ((int8_t)rd8(m, obj + 0x3Cu) < 3) {
                    if (rd8(m, obj + 0x3Du) == 0u) {
                        (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x1020A000u);
                        (void)Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x10208000u);
                        r->d[0] = Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x10206000u);
                    }
                    wr16(m, obj + 0x3Cu, (uint16_t)(rd16(m, obj + 0x3Cu) + 0x0080u));
                }

                wr32(m, obj + 0x00u, 0x0013BA06u);      /* 0x13B9FE */
                return 0;
            }
        }
    }

    /* 0x13BA38 */
    {
        uint16_t c6 = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, c6);
        if ((int16_t)c6 > 0) {
            uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
            r->a[1] = a1;
            uint16_t ang = sub_0CC3C6(m, obj, a1, &r->d[0], &r->d[1]);
            wr16(m, obj + 0x20u, ang);
            wr16(m, obj + 0x42u, 4u);
            wr16(m, obj + 0x22u, 0x0078u);

            if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)
                goto die_explode;
            if ((int32_t)rd32(m, obj + 0x24u) <= 0)
                goto die_score;

            r->a[1] = 0x0013BC34u;
            sub_0CBD52(m, obj, 0x0013BC34u, &r->d[0], &r->d[1]);

            if (on_screen_bomb(m, obj)) {
                relay_explode(m, r);
                return 0;
            }

            move_by_angle_regs(m, r, rd16(m, obj + 0x20u));
            Obj_11C32C(m, obj);
            draw_sprite_regs(m, r);
            sub_0CBDA4(m, obj, &r->a[1]);
            wr32(m, obj + 0x00u, 0x0013BABEu);          /* 0x13BAB6 */
            return 0;
        }
    }

    /* 0x13BACE */
    wr16(m, obj + 0x42u, 0u);
    wr8(m, obj + 0x22u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)        /* 0x13BADE */
        goto die_explode;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)
        goto die_score;

    if (rd8(m, obj + 0x22u) == 0u) {                    /* 0x13BAF0 */
        r->a[1] = 0x0013BC4Eu;
        sub_0CBD24(m, obj, 0x0013BC4Eu, &r->d[0], &r->d[1]);
        if ((int16_t)rd16(m, 0x0013BC4Eu + (int16_t)r->d[0]) > 0)
            goto tail_13bb6c;
        wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
        wr8(m, obj + 0x22u, 1u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
    }

    {
        uint32_t idx = rd8(m, obj + 0x3Au) & 0x0Fu;     /* 0x13BB1E */
        wr32(m, obj + 0x10u, rd32(m, 0x0013BC6Eu + idx * 4u));
        uint16_t step = rd16(m, obj + 0x3Eu);
        r->d[0] = setw(r->d[0], step);
        wr16(m, obj + 0x3Au, (uint16_t)(rd16(m, obj + 0x3Au) + step));
        uint16_t spd = (uint16_t)(rd16(m, obj + 0x3Eu) + 4u);
        wr16(m, obj + 0x3Eu, spd);
        if ((int16_t)spd > 0x0100) {
            wr16(m, obj + 0x3Eu, 0x0100u);
            wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u));
            if ((int8_t)rd8(m, obj + 0x22u) <= 1) {
                wr8(m, obj + 0x22u, 2u);
                uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);
                r->a[1] = a1;
                uint16_t ang = sub_0CC3C6(m, obj, a1, &r->d[0], &r->d[1]);
                wr16(m, obj + 0x20u, ang);
            }
        }
    }

tail_13bb6c:
    if (on_screen_bomb(m, obj)) {
        relay_explode(m, r);
        return 0;
    }

    move_by_angle_regs(m, r, rd16(m, obj + 0x20u));     /* 0x13BB90 */
    wr8(m, obj + 0x35u, 0u);                            /* 0x13BB9A */
    Obj_11C32C(m, obj);                                 /* 0x13BB9E */
    draw_sprite_regs(m, r);                             /* 0x13BBA4 */
    sub_0CBDA4(m, obj, &r->a[1]);                       /* 0x13BBAA */

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        int16_t y = (int16_t)rd16(m, obj + 0x18u);
        if (x <= -0x0040 || x >= 0x0180 || y <= -0x0040 || y >= 0x0128) {
            Obj_Free(m, obj);                           /* 0x13BC26 */
            return 0;
        }
    }

    wr32(m, obj + 0x00u, 0x0013BBE0u);                  /* 0x13BBD8 */
    return 0;

die_score:
    r->a[1] = Obj_0CBDDE(m, obj);
    r->d[0] = rd16(m, obj + 0x4Cu);
    wr32(m, r->a[1] + 0x48u, rd32(m, r->a[1] + 0x48u) + r->d[0]);

die_explode:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20180002u);
    Sound_PlayHooked(m, 0x0035u);
    Obj_Free(m, obj);
    return 0;
}
