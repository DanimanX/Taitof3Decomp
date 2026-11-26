/* ROM 0x13C3AA: boss/enemy spinning launch/orbit tick (A0 = object).
 * On A5-0x144B bit 4, spawns explosion 0x13E4EE (via 0x13E4D0), plays sound
 * 0x35, and frees self (0x0105EC). If A5+0x1D58 bit 1 set and -0x0A <= X <=
 * 0x11C, awards +0x4C score to A5+0x1D50 and jumps to 0x0CBB2C. Otherwise sets
 * speed +0x42 = +0x06 >> 2, moves by angle +0x20 (0x0CBE12), draws sprite
 * (0x001282), appends to collision lists (0x0CBDA4), decrements +0x06 and
 * re-arms 0x13C3AA while > 0; else resets fields, picks random +0x22/+0x23 via
 * Rng_NextMem, offsets (-8, -8), and transitions to 0x13C43E. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
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

int Fn_13C3AA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x13C3AA..0x13C3B0 */
        uint32_t d0 = Obj_13E4D0(m, obj, 2u);            /* 0x13C57C..0x13C57E */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, d0); /* 0x13C584..0x13C59A */
        Sound_PlayHooked(m, 0x0035u);                    /* 0x13C59E..0x13C5AE */
        Obj_Free(m, obj);                                /* 0x13C5B2..0x13C5BE */
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {       /* 0x13C3B4 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {               /* 0x13C3BC..0x13C3CA */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);      /* 0x0CBD12 */
            r->a[1] = a1;
            uint32_t d0 = (uint32_t)rd16(m, obj + 0x4Cu);
            r->d[0] = d0;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
            explode_tick_regs(m, r);                     /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x06u) >> 2)); /* 0x13C3D2..0x13C3D8 */
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));       /* 0x13C3DC..0x13C3E0 */
    draw_sprite_regs(m, r);                              /* 0x13C3E6 */
    sub_0CBDA4(m, obj, &r->a[1]);                        /* 0x13C3EC */

    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x06u) - 1u); /* 0x13C3F2 */
    wr16(m, obj + 0x06u, cnt);
    if ((int16_t)cnt > 0) {                              /* 0x13C3F6 */
        wr32(m, obj, 0x0013C3AAu);                       /* 0x13C3A2 */
        return 0;
    }

    wr16(m, obj + 0x3Au, 0u);                            /* 0x13C3F8 */
    wr16(m, obj + 0x3Eu, 0u);                            /* 0x13C3FC */
    wr8(m, obj + 0x1Cu, 0u);                             /* 0x13C400 */
    wr8(m, obj + 0x35u, 0u);                             /* 0x13C404 */
    wr16(m, obj + 0x42u, 0u);                            /* 0x13C408 */

    uint32_t d0 = Rng_NextMem(m);                        /* 0x13C40C */
    wr8(m, obj + 0x22u, (d0 & 0x80000000u) ? 0xFFu : 0x00u); /* 0x13C412..0x13C416 */
    d0 = setw(d0, (uint16_t)((d0 & 0x000Fu) + 0x0010u)); /* 0x13C41A..0x13C41E */
    wr8(m, obj + 0x23u, (uint8_t)d0);                    /* 0x13C422 */
    wr16(m, obj + 0x06u, 0u);                            /* 0x13C426 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 8u)); /* 0x13C42A */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u)); /* 0x13C430 */
    wr32(m, obj, 0x0013C43Eu);                           /* 0x13C436 */
    r->d[0] = d0;
    return 0;
}
