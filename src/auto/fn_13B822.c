/* ROM 0x13B822: scrolling enemy wave/formation tick (A0 = object).
 * On A5-0x144B bit 4, spawns 0x13E4EE explosion (0x20180002), plays sound
 * 0x35 (0x13F238+0x0039D8) and frees self (0x0105EC). If HP (+0x24) <= 0,
 * awards +0x4C to score receiver (0x0CBDDE) before the same death sequence.
 * Otherwise animates from 0x13BC34 (0x0CBD52), handles bomb-clear (0x0CBD12
 * -> 0x0CBB2C), steps X by -2.0, updates hit flash (0x11C32C), draws sprite
 * (0x001282), appends to collision lists (0x0CBDA4), and counts down +0x06
 * to spawn 0x13B8B4 (group 5, count 2) and transition to 0x13B8A6. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
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

int Fn_13B822(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)        /* 0x13B822 */
        goto death_spawn;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x13B82C */
        uint32_t a1 = Obj_0CBDDE(m, obj);               /* 0x13BBE4 */
        uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);   /* 0x13BBEA..0x13BBEC */
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);  /* 0x13BBF0 */
        r->a[1] = a1;
        r->d[0] = d0;
        goto death_spawn;
    }

    Obj_Animate(m, obj, 0x0013BC34u);                   /* 0x13B834..0x13B838 */
    r->d[0] = setw(r->d[0], rd16(m, obj + 0x3Au));
    r->d[1] = setw(r->d[1], rd16(m, obj + 0x3Eu));
    r->a[1] = 0x0013BC34u;

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {      /* 0x13B83E */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {              /* 0x13B846..0x13B854 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));   /* 0x13B856 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);     /* 0x0CBD12 */
            uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            r->a[1] = a1;
            r->d[0] = d0;
            explode_tick_regs(m, r);                    /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x00020000u); /* 0x13B862 */
    Obj_11C32C(m, obj);                                 /* 0x13B86A */
    draw_sprite_regs(m, r);                             /* 0x13B870 */

    {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);         /* 0x13B876 -> 0x0CBDA4 */
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        p2 += 4u;
        wr32(m, F3_A5 + 0x0906u, p2);
        r->a[1] = p2;
    }

    {
        int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x13B87C */
        wr16(m, obj + 0x06u, (uint16_t)cnt);
        if (cnt > 0) {                                  /* 0x13B880 */
            wr32(m, obj, 0x0013B822u);                  /* 0x13B81A */
            return 0;
        }
    }

    r->d[0] = Obj_Spawn(m, obj, 0x0013B8B4u, 5u, 2u, 0u); /* 0x13B882..0x13B894 */
    wr32(m, obj, 0x0013B8A6u);                          /* 0x13B89E */
    return 0;

death_spawn:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 0x20180002u); /* 0x13BBF4..0x13BC08 */
    Sound_PlayHooked(m, 0x0035u);                       /* 0x13BC12..0x13BC1C */
    Obj_Free(m, obj);                                   /* 0x13BC26..0x13BC28 */
    return 0;
}
