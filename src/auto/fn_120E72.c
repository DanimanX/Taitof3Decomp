/* ROM 0x120E72: homing/tracking enemy tick (A0 = object, A0+0x08 = parent).
 * If HP (+0x24) <= 0, awards +0x4C to score receiver (0x0CBDDE), plays sound
 * 0x35, spawns 0x13E4EE explosion (with 0x13E4D0 center offset), and transitions
 * to 0x1210D2 (Obj_Free). Same death/spawn happens on A5-0x144B bit 4.
 * Otherwise steers (+0x21) toward parent + (rng&0x1F - 0x32, 0), switches
 * anim table offset (+0x3A) when (+0x20 & 0x7F) == 0x40, moves via 0x0CBE12,
 * appends to collision lists (0x0CBDA4), steps 0x1210E0 anim (0x0CBD24),
 * handles bomb-clear (0x0CBD12 -> 0x0CBB2C), updates flash +0x1D (0x11C32C)
 * and draws sprite (0x001282). Frees if X <= -0x10; when +0x06 countdown
 * reaches 0 and +0x1C == 0, aims at player (0x0CC33A), sets anim 0x39090,
 * sets speed/angle (+0x22, +0x42 via 0x0CBEB0), and transitions to 0x120FD6. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
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

/* ROM 0x0CBEB0: difficulty-scaled speed word (A5+0x87E). */
static uint32_t sub_0CBEB0(const f3mem_t *m, uint16_t d0w, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(((int16_t)rd16(m, F3_A5 + 0x087Eu) << 3) + 0x0C00);
    *d1_out = setw(*d1_out, d1w);
    uint32_t prod = (uint32_t)((int32_t)(int16_t)d0w * (int32_t)(int16_t)d1w);
    prod <<= 4;
    return (prod << 16) | (prod >> 16);
}

/* ROM 0x0CC33A: angle from obj+0x14/0x18 + (d0w, d1w) toward player A5+0x1D28. */
static uint16_t sub_0CC33A(const f3mem_t *m, uint32_t obj, uint16_t d0w, uint16_t d1w,
                           uint32_t *a1_out)
{
    uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);
    if (tgt == 0u) {
        *a1_out = 0xFFFFFFFFu;
        return 0x0080u;
    }
    *a1_out = tgt;
    int16_t cx = (int16_t)((int16_t)rd16(m, tgt + 0x2Cu) + (int16_t)rd16(m, tgt + 0x2Eu)) >> 1;
    int16_t cy = (int16_t)((int16_t)rd16(m, tgt + 0x30u) + (int16_t)rd16(m, tgt + 0x32u)) >> 1;
    uint16_t dx = (uint16_t)((uint16_t)(0u - d0w) + (uint16_t)cx + rd16(m, tgt + 0x14u) - rd16(m, obj + 0x14u));
    uint16_t dy = (uint16_t)((uint16_t)(0u - d1w) + (uint16_t)cy + rd16(m, tgt + 0x18u) - rd16(m, obj + 0x18u));
    return Obj_Atan2(m, dx, dy);
}

int Fn_120E72(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {           /* 0x120E72 */
        a1 = Obj_0CBDDE(m, obj);                        /* 0x121084 */
        d0 = (uint32_t)rd16(m, obj + 0x4Cu);
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + d0);
        goto death_spawn;
    }
    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)        /* 0x120E7A */
        goto death_spawn;

    /* 0x120E84..0x120EC6: aim steering toward parent + random offset */
    {
        uint32_t parent = rd32(m, obj + 0x08u);
        uint32_t rng = Rng_NextMem(m);
        uint16_t dx = (uint16_t)((rng & 0x001Fu) + rd16(m, parent + 0x14u) - 0x0032u - rd16(m, obj + 0x14u));
        uint16_t dy = (uint16_t)(rd16(m, parent + 0x18u) - rd16(m, obj + 0x18u));
        uint16_t ang = Obj_Atan2(m, dx, dy);
        int8_t diff = (int8_t)(uint8_t)(ang - rd16(m, obj + 0x20u));
        if (diff >= 0)
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) + 1u));
        else
            wr8(m, obj + 0x21u, (uint8_t)(rd8(m, obj + 0x21u) - 1u));
    }

    if ((rd16(m, obj + 0x20u) & 0x007Fu) == 0x0040u) {  /* 0x120EC6 */
        wr16(m, obj + 0x3Eu, 0u);
        uint32_t rng = Rng_NextMem(m);
        wr16(m, obj + 0x3Au, ((rng & 1u) == 0u) ? 0x001Au : 0x0036u);
    }

    /* 0x120EF8..0x120F3A */
    Obj_MoveByAngle(m, obj, rd16(m, obj + 0x20u));      /* 0x120EFC */
    {
        uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);         /* 0x120F02 -> 0x0CBDA4 */
        wr32(m, p1, obj);
        wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
        uint32_t p2 = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p2, obj);
        a1 = p2 + 4u;
        wr32(m, F3_A5 + 0x0906u, a1);
    }

    Obj_AnimFrameStep(m, obj, 0x001210E0u);             /* 0x120F0E */
    {
        int16_t off = (int16_t)rd16(m, obj + 0x3Au);
        int16_t dur = (int16_t)rd16(m, 0x001210E0u + (uint32_t)(int32_t)off);
        if (dur == 0) {
            wr8(m, obj + 0x1Cu, rd8(m, obj + 0x1Cu) ^ 1u);
            wr16(m, obj + 0x3Au, (uint16_t)(off + 2));
            wr16(m, obj + 0x3Eu, 0u);
        } else if (dur < 0) {
            wr16(m, obj + 0x3Au, 0u);
            wr16(m, obj + 0x3Eu, 0u);
        }
    }

    /* 0x120F3A..0x120F5E: bomb-clear check */
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x0A && x <= 0x011C) {
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));
            Score_AddCurrent(m, (uint32_t)rd16(m, obj + 0x4Cu)); /* 0x0CBD12 */
            r->d[0] = (uint32_t)rd16(m, obj + 0x4Cu);
            r->a[1] = rd32(m, F3_A5 + 0x1D50u);
            explode_tick_regs(m, r);
            return 0;
        }
    }

    /* 0x120F5E..0x120F84 */
    Obj_11C32C(m, obj);
    draw_sprite_regs(m, r);
    d0 = r->d[0];
    d1 = r->d[1];
    a1 = r->a[1];

    if ((int16_t)rd16(m, obj + 0x14u) <= -0x0010) {     /* 0x120F6A */
        Obj_Free(m, obj);                               /* 0x1210D2 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    {
        int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);
        wr16(m, obj + 0x06u, (uint16_t)cnt);
        if (cnt > 0 || rd8(m, obj + 0x1Cu) != 0u) {
            wr32(m, obj, 0x00120E72u);                  /* 0x120E6A */
            r->d[0] = d0;
            r->d[1] = d1;
            r->a[1] = a1;
            return 0;
        }
    }

    /* 0x120F84..0x120FD4: launch/dive transition to 0x120FD6 */
    {
        uint16_t ang = sub_0CC33A(m, obj, 0x000Cu, 0x000Cu, &a1);
        wr16(m, obj + 0x20u, ang);
        wr32(m, obj + 0x10u, 0x00039090u);
        wr16(m, obj + 0x3Au, 0u);
        wr16(m, obj + 0x3Eu, 0u);
        uint32_t spd = sub_0CBEB0(m, 0x001Eu, &d1);
        wr16(m, obj + 0x22u, (uint16_t)spd);
        wr16(m, obj + 0x06u, 0x000Eu);
        wr16(m, obj + 0x42u, (uint16_t)-0x001E);
        uint32_t acc = sub_0CBEB0(m, 0x0010u, &d1);
        d0 = acc;
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + (uint16_t)acc));
        wr32(m, obj, 0x00120FD6u);
    }

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;

death_spawn:
    Sound_PlayHooked(m, 0x0035u);                       /* 0x121094 */
    d0 = Obj_13E4D0(m, obj, 2u);                        /* 0x1210A8 */
    d0 = Obj_Spawn(m, obj, 0x0013E4EEu, 0u, 1u, d0);
    wr32(m, obj, 0x001210D2u);                          /* 0x1210CA */
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
