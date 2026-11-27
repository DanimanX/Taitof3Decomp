/* ROM 0x120AEC: falling/rising enemy hazard tick (A0 = object).
 * Switches to follow-up record 0x0CD444[0] (0x0CD3D6 -> 0x0CD412) if HP <= 0
 * or bomb-clear (A5-0x144B bit 4). Handles on-screen bomb/explode (0x0CBB2C)
 * when A5+0x1D58 bit 1 set and X in [-10, 0x11C]. Otherwise moves along +0x20.w
 * (0x0CBE12), appends to hazard list (A5+0x906), draws sprite (0x001282),
 * re-arms 0x120AEC if Y > -0x10, else sets timer +0x06 = 0x3C + (rng & 0xF)
 * and transitions to 0x120B44 (or 0x120B74 if timer == 0). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
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

/* ROM 0x0CBE12 with D0/D1/D2/A3 register tracking. */
void FnAuto_MoveByAngle(const f3mem_t *m, f3regs_t *r, uint16_t ang)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int32_t off = (int32_t)(int16_t)ang * 2;
    int16_t cos_v = (int16_t)rd16(m, (uint32_t)(0x00027E40 + 0x80 + off));
    int16_t sin_v = (int16_t)rd16(m, (uint32_t)(0x00027E40 + off));
    int32_t vx = (int32_t)speed * (int32_t)cos_v;
    int32_t vy = (int32_t)speed * (int32_t)sin_v;

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + (uint32_t)vx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + (uint32_t)vy);

    r->d[0] = setw(r->d[0], ang);
    r->d[1] = (uint32_t)vx;
    r->d[2] = (uint32_t)vy;
    r->a[3] = 0x00027E40u;
}

int Fn_120AEC(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0 ||             /* 0x120AEC */
        (rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {        /* 0x120AF4 */
        uint32_t a1 = rd32(m, 0x000CD444u);               /* 0x120BB0 -> 0x0CD3D6 (D0w=0) */
        uint16_t cmd = rd16(m, a1);
        a1 += 2u;
        if (cmd != 0u)
            Sound_PlayHooked(m, cmd);
        wr32(m, obj + 0x0Cu, a1);                         /* 0x0CD3F6 */
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        wr16(m, obj + 0x1Eu, 0u);
        wr16(m, obj + 0x1Cu, 0u);
        wr32(m, obj, 0x000CD412u);                        /* 0x0CD40A */
        r->d[0] = setw(r->d[0], cmd);
        r->a[1] = a1;
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {        /* 0x120AFE */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {                /* 0x120B06..0x120B14 */
            explode_tick_regs(m, r);                      /* 0x120B16 */
            return 0;
        }
    }

    /* 0x120B1C -> 0x120BBA */
    FnAuto_MoveByAngle(m, r, rd16(m, obj + 0x20u));       /* 0x120BBA..0x120BBE */
    uint32_t p = rd32(m, F3_A5 + 0x0906u);                /* 0x120BC4 */
    wr32(m, p, obj);                                      /* 0x120BC8 */
    wr32(m, F3_A5 + 0x0906u, p + 4u);                     /* 0x120BCA */
    draw_sprite_regs(m, r);                               /* 0x120BCE */

    if ((int16_t)rd16(m, obj + 0x18u) > -0x0010) {        /* 0x120B20..0x120B26 */
        wr32(m, obj, 0x00120AECu);                        /* 0x120AE4 */
        return 0;
    }

    uint32_t d0 = Rng_NextMem(m);                         /* 0x120B28 */
    uint16_t d0w = (uint16_t)((d0 & 0x000Fu) + 0x003Cu);  /* 0x120B2E..0x120B32 */
    wr16(m, obj + 0x06u, d0w);                            /* 0x120B36 */
    wr32(m, obj, 0x00120B44u);                            /* 0x120B3C (d0w in [0x3C..0x4B] != 0) */
    r->d[0] = setw(d0, d0w);
    return 0;
}
