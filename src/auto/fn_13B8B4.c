/* ROM 0x13B8B4: initialize and run first tick of multi-shot enemy/minion
 * spawned from parent (+0x08). Copies transform/hitbox/HP/anim/score/palette
 * from parent, picks random angle offset in [-0x20..0x1F] via Rng_NextMem,
 * sets +0x06=4, +0x22=0x1400, +0x3C=0. Handles bomb-clear (A5-0x144B bit 4),
 * HP <= 0 (score via 0x0CBDDE, spawn 0x13E4EE explosion, Sound_PlayHooked 0x35,
 * Obj_Free), and on-screen bomb/explode (0x0CBD12 -> 0x0CBB2C). Otherwise
 * animates (0x13BC34), moves by angle (0x0CBE12), updates flash (0x11C32C),
 * draws sprite (0x001282), appends to collision lists (0x0CBDA4), spawns
 * three 0x0CCD5C children on +0x3D==0 while (int8_t)+0x3C < 3, and arms
 * 0x13BA06. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
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

/* ROM 0x0CBE12 with D1 register tracking. */
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

    r->d[1] = (uint32_t)vx;
}

int Fn_13B8B4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                 /* 0x13B8B4 */

    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u));          /* 0x13B8B8 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));          /* 0x13B8BE */
    wr32(m, obj + 0x2Cu, rd32(m, parent + 0x2Cu));          /* 0x13B8C4 */
    wr32(m, obj + 0x30u, rd32(m, parent + 0x30u));          /* 0x13B8CA */
    wr32(m, obj + 0x28u, rd32(m, parent + 0x28u));          /* 0x13B8D0 */
    wr32(m, obj + 0x24u, rd32(m, parent + 0x24u));          /* 0x13B8D6 */
    wr16(m, obj + 0x3Au, rd16(m, parent + 0x3Au));          /* 0x13B8DC */
    wr16(m, obj + 0x3Eu, rd16(m, parent + 0x3Eu));          /* 0x13B8E2 */
    wr16(m, obj + 0x1Cu, rd16(m, parent + 0x1Cu));          /* 0x13B8E8 */
    wr16(m, obj + 0x34u, rd16(m, parent + 0x34u));          /* 0x13B8EE */
    wr16(m, obj + 0x4Cu, rd16(m, parent + 0x4Cu));          /* 0x13B8F4 */
    wr8(m, obj + 0x4Fu, rd8(m, parent + 0x4Fu));            /* 0x13B8FA */
    wr16(m, obj + 0x1Eu, 0u);                               /* 0x13B900 */
    r->a[1] = parent;

    {
        uint32_t d0 = Rng_NextMem(m);                       /* 0x13B904 */
        d0 &= 0xFFFF003Fu;                                  /* 0x13B90A */
        d0 = (d0 & 0xFFFFFF00u) | (uint8_t)(d0 - 0x20u);    /* 0x13B90E */
        wr16(m, obj + 0x20u, (uint16_t)d0);                 /* 0x13B912 */
        r->d[0] = d0;
    }

    wr16(m, obj + 0x06u, 4u);                               /* 0x13B916 */
    wr16(m, obj + 0x22u, 0x1400u);                          /* 0x13B91C */
    wr16(m, obj + 0x3Cu, 0u);                               /* 0x13B922 */

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)            /* 0x13B926 */
        goto die_explode;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* 0x13B930 */
        uint32_t recv = Obj_0CBDDE(m, obj);                 /* 0x13BBE4 */
        r->a[1] = recv;
        uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);      /* 0x13BBEA..0x13BBEC */
        r->d[0] = pts;
        wr32(m, recv + 0x48u, rd32(m, recv + 0x48u) + pts); /* 0x13BBF0 */
        goto die_explode;
    }

    r->a[1] = 0x0013BC34u;                                  /* 0x13B938 */
    Obj_Animate(m, obj, 0x0013BC34u);                       /* 0x13B93C */
    r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {          /* 0x13B942 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {                  /* 0x13B94A..0x13B958 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x13B95A */
            uint32_t a1_exp = rd32(m, F3_A5 + 0x1D50u);     /* 0x0CBD12 */
            r->a[1] = a1_exp;
            uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);
            r->d[0] = pts;
            wr32(m, a1_exp + 0x48u, rd32(m, a1_exp + 0x48u) + pts);
            explode_tick_regs(m, r);                        /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    {
        uint16_t spd = (uint16_t)(int16_t)(int8_t)rd8(m, obj + 0x22u); /* 0x13B966..0x13B96A */
        wr16(m, obj + 0x42u, spd);                          /* 0x13B96C */
        uint16_t ang = rd16(m, obj + 0x20u);                /* 0x13B970 */
        r->d[0] = setw(r->d[0], ang);
        move_by_angle_regs(m, r, ang);                      /* 0x13B974 */
    }

    Obj_11C32C(m, obj);                                     /* 0x13B97A */
    draw_sprite_regs(m, r);                                 /* 0x13B980 */
    sub_0CBDA4(m, obj, &r->a[1]);                           /* 0x13B986 */

    if ((int8_t)rd8(m, obj + 0x3Cu) < 3) {                  /* 0x13B98C */
        if (rd8(m, obj + 0x3Du) == 0u) {                    /* 0x13B996 */
            r->d[0] = Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x1020A000u); /* 0x13B99E..0x13B9B8 */
            r->d[0] = Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x10208000u); /* 0x13B9BC..0x13B9D6 */
            r->d[0] = Obj_Spawn(m, obj, 0x000CCD5Cu, 9u, 2u, 0x10206000u); /* 0x13B9DA..0x13B9F4 */
        }
        wr16(m, obj + 0x3Cu, (uint16_t)(rd16(m, obj + 0x3Cu) + 0x0080u)); /* 0x13B9F8 */
    }

    wr32(m, obj + 0x00u, 0x0013BA06u);                      /* 0x13B9FE */
    return 0;

die_explode:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20180002u); /* 0x13BBF4..0x13BC0E */
    Sound_PlayHooked(m, 0x0035u);                           /* 0x13BC12..0x13BC22 */
    Obj_Free(m, obj);                                       /* 0x13BC26..0x13BC2E */
    return 0;
}
