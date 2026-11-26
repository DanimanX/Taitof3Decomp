/* ROM 0x130F58: homing missile/projectile tick (A0 = object).
 * Frees itself via 0x0105EC if bomb-clear (A5-0x144B bit 4), HP <= 0, or
 * animation 0x13100E finishes (0x0CBD24 <= 0). Checks on-screen bomb/explode
 * (0x0CBB2C). Otherwise moves along +0x20.b angle (0x0CBE12), steers +0x20.b
 * by +/-1 toward (A5+0x30E2)->+0x18, decrements +0x06 and appends to hazard
 * list (0x0CBDAE) while > 0, draws sprite (0x001282), frees outside
 * [-0x20..0x140] x [-0x20..0xE8], else re-arms 0x130F58. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
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

int Fn_130F58(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)        /* 0x130F58 */
        goto free_obj;
    if ((int32_t)rd32(m, obj + 0x24u) <= 0)             /* 0x130F62 */
        goto free_obj;

    r->a[1] = 0x0013100Eu;                              /* 0x130F6A */
    sub_0CBD24(m, obj, 0x0013100Eu, &r->d[0], &r->d[1]); /* 0x130F70 */
    if ((int16_t)rd16(m, 0x0013100Eu + (int16_t)r->d[0]) <= 0) /* 0x130F76 */
        goto free_obj;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x130F7E */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {
            explode_tick_regs(m, r);                    /* 0x130F96 -> 0x0CBB2C */
            return 0;
        }
    }

    {
        uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8); /* 0x130F9C */
        r->d[0] = setw(r->d[0], ang);
        move_by_angle_regs(m, r, ang);                  /* 0x130FA2 */
    }

    {
        uint32_t tgt = rd32(m, F3_A5 + 0x30E2u);        /* 0x130FA8 */
        r->a[1] = tgt;
        uint16_t ty = rd16(m, tgt + 0x18u);             /* 0x130FAC */
        r->d[0] = setw(r->d[0], ty);
        if ((int16_t)ty <= (int16_t)rd16(m, obj + 0x18u)) /* 0x130FB0 */
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + 0x0100u));
        else
            wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) - 0x0100u));
    }

    {
        uint16_t c6 = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x130FC8 */
        wr16(m, obj + 0x06u, c6);
        if ((int16_t)c6 > 0) {
            uint32_t a1 = rd32(m, F3_A5 + 0x0906u);     /* 0x130FD0 -> 0x0CBDAE */
            wr32(m, a1, obj);
            a1 += 4u;
            wr32(m, F3_A5 + 0x0906u, a1);
            r->a[1] = a1;
        }
    }

    draw_sprite_regs(m, r);                             /* 0x130FD6 */

    {
        uint16_t x = rd16(m, obj + 0x14u);              /* 0x130FDC */
        r->d[1] = setw(r->d[1], x);
        if ((int16_t)x < -0x0020 || (int16_t)x > 0x0140)
            goto free_obj;

        uint16_t y = rd16(m, obj + 0x18u);              /* 0x130FEC */
        r->d[1] = setw(r->d[1], y);
        if ((int16_t)y < -0x0020 || (int16_t)y > 0x00E8)
            goto free_obj;
    }

    wr32(m, obj + 0x00u, 0x00130F58u);                  /* 0x130FFC -> 0x130F50 */
    return 0;

free_obj:
    Obj_Free(m, obj);                                   /* 0x131000 */
    return 0;
}
