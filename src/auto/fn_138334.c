/* ROM 0x138334: detached boss turret/pod attack tick (A0 = object).
 * Branches into 0x13823C: if bomb-clear (A5-0x144B bit 4) or +0x24 <= 0
 * (awarding +0x4C to score receiver via 0x0CBDDE), spawns 0x13E4EE (grp 5,
 * cnt 2, arg 2), plays sound 0x40 via Sound_PlayHooked, and sets +0x00 =
 * 0x138382. If A5+0x1D58 bit 1 set and +0x14 in [-0x0A, 0x11C], copies +0x4F
 * to +0x1D, adds +0x4C to (A5+0x1D50)->+0x48, and jumps to 0x0CBB2C.
 * Otherwise ramps +0x06 by +0x10 (clamped at 0x800, incrementing +0x42 and
 * aiming +0x23 at A5+0x1D28 via 0x0CC3C6 on first latch), moves along +0x23
 * (0x0CBE12), spins +0x20 by +0x06, updates sprite/lists via 0x138390, fires
 * twin 0x0CCD04 shots when 0x0CC2BC(0x14) == 0, and frees via 0x138382
 * (0x0105EC) if outside [-32, 320] x [-32, 232] (0x0CC112/0x0CC116) else
 * re-arms 0x138334. */
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

/* ROM 0x0CBE12: pos += speed * {cos,sin}[angle]; D1 = speed * cos[angle], D2 = speed * sin[angle]. */
static void move_by_angle_regs(const f3mem_t *m, f3regs_t *r, uint16_t angle)
{
    uint32_t obj = r->a[0];
    int16_t speed = (int16_t)rd16(m, obj + 0x42u);
    int16_t c = (int16_t)rd16(m, 0x027E40u + 0x80u + 2u * angle);
    int16_t s = (int16_t)rd16(m, 0x027E40u + 2u * angle);
    uint32_t dx = (uint32_t)((int32_t)speed * (int32_t)c);
    uint32_t dy = (uint32_t)((int32_t)speed * (int32_t)s);
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + dx);
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + dy);
    r->d[1] = dx;
    r->d[2] = dy;
    r->a[3] = 0x00027E40u;
}

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem of (A5+0x878) / max(1, ((0x180 - A5+0x87E)*d0w)>>16). */
static uint16_t sub_0CC2BC(const f3mem_t *m, f3regs_t *r, uint16_t d0w)
{
    uint16_t diff = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = ((uint32_t)diff * (uint32_t)d0w) << 8;
    d1 = (d1 << 16) | (d1 >> 16);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t div_in = (uint32_t)rd16(m, F3_A5 + 0x0878u);
    uint16_t d1w = (uint16_t)d1;
    uint32_t q = div_in / d1w;
    uint32_t rem = div_in % d1w;
    uint32_t d0 = (q <= 0xFFFFu) ? ((q << 16) | rem) : div_in;
    r->d[0] = d0;
    r->d[1] = d1;
    return (uint16_t)d0;
}

/* ROM 0x138390: pick sprite/flip from (+0x20 + 0x400), update flash, draw & add to lists. */
static void sub_138390(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t w = (uint16_t)(rd16(m, obj + 0x20u) + 0x0400u); /* 0x138394..0x138398 */
    uint32_t idx1 = (w >> 14) & 0x03u;                        /* 0x13839C bfextu {16:2} */
    uint32_t idx0 = (w >> 11) & 0x0Fu;                        /* 0x1383A0 bfextu {17:4} */
    wr32(m, obj + 0x10u, rd32(m, 0x00138412u + idx0 * 4u));   /* 0x1383A4 */
    wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + idx1));          /* 0x1383AA..0x1383B0 */
    r->d[0] = idx0;
    r->d[1] = idx1;
    r->a[1] = 0x000CC10Eu;

    Obj_11C32C(m, obj);                                       /* 0x1383B6 */

    int16_t x = (int16_t)(rd16(m, obj + 0x14u) - 0x0010);     /* 0x1383BC */
    int16_t y = (int16_t)(rd16(m, obj + 0x18u) - 0x0010);     /* 0x1383C2 */
    wr16(m, obj + 0x14u, (uint16_t)x);
    wr16(m, obj + 0x18u, (uint16_t)y);

    if (x > -0x0020 && y > -0x0020 && x < 0x0140 && y < 0x00E8) { /* 0x1383C8..0x1383EC */
        draw_sprite_regs(m, r);                               /* 0x1383F0 */
        if (rd8(m, F3_A5 + 0x30F3u) == 0u) {                  /* 0x1383F6 */
            uint32_t p1 = rd32(m, F3_A5 + 0x08FEu);           /* 0x0CBDA4 */
            wr32(m, p1, obj);
            wr32(m, F3_A5 + 0x08FEu, p1 + 4u);
            uint32_t p2 = rd32(m, F3_A5 + 0x0906u);           /* 0x0CBDAE */
            wr32(m, p2, obj);
            r->a[1] = p2 + 4u;
            wr32(m, F3_A5 + 0x0906u, r->a[1]);
        }
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u)); /* 0x138404 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u)); /* 0x13840A */
}

int Fn_138334(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {            /* 0x13823C */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u); /* 0x138348 */
        Sound_PlayHooked(m, 0x0040u);                         /* 0x138366 */
        wr32(m, obj, 0x00138382u);                            /* 0x13837A */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {                 /* 0x138246 */
        uint32_t a1 = Obj_0CBDDE(m, obj);                     /* 0x138338 */
        r->a[1] = a1;
        wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u); /* 0x138348 */
        Sound_PlayHooked(m, 0x0040u);                         /* 0x138366 */
        wr32(m, obj, 0x00138382u);                            /* 0x13837A */
        return 0;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {            /* 0x13824E */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {                    /* 0x138256..0x138264 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));         /* 0x138266 */
            uint32_t a1 = rd32(m, F3_A5 + 0x1D50u);           /* 0x0CBD12 */
            r->a[1] = a1;
            wr32(m, a1 + 0x48u, rd32(m, a1 + 0x48u) + (uint32_t)rd16(m, obj + 0x4Cu));
            explode_tick_regs(m, r);                          /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 0x0010u)); /* 0x138272 */
    if ((int16_t)rd16(m, obj + 0x06u) > 0x0800) {             /* 0x138278 */
        wr16(m, obj + 0x06u, 0x0800u);                        /* 0x138282 */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u)); /* 0x138288 */
        uint8_t fl = rd8(m, obj + 0x22u);                     /* 0x13828C bset #0 */
        wr8(m, obj + 0x22u, (uint8_t)(fl | 0x01u));
        if ((fl & 0x01u) == 0u) {
            uint32_t a1 = rd32(m, F3_A5 + 0x1D28u);           /* 0x138296 */
            r->a[1] = a1;
            uint16_t ang = Obj_0CC3C6(m, obj, a1);            /* 0x13829A */
            r->d[0] = setw(r->d[0], ang);
            wr8(m, obj + 0x23u, (uint8_t)ang);                /* 0x1382A0 */
        }
    }

    uint32_t ang = (uint32_t)rd8(m, obj + 0x23u);             /* 0x1382A4..0x1382A6 */
    r->d[0] = ang;
    move_by_angle_regs(m, r, (uint16_t)ang);                  /* 0x1382AA */
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, obj + 0x06u))); /* 0x1382B0 */
    sub_138390(m, r);                                         /* 0x1382B8 */

    if (sub_0CC2BC(m, r, 0x0014u) == 0u) {                    /* 0x1382BC..0x1382C8 */
        uint32_t d1 = (uint32_t)rd16(m, obj + 0x20u) & 0xFF00u; /* 0x1382CC..0x1382D2 */
        (void)Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1);     /* 0x1382E4 */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)(d1 - 0x8000u);   /* 0x1382EE */
        r->d[1] = d1;
        r->d[0] = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1); /* 0x138302 */
    }

    int16_t x = (int16_t)rd16(m, obj + 0x14u);                /* 0x13830C */
    r->d[0] = setw(r->d[0], (uint16_t)x);
    if (x < (int16_t)rd16(m, 0x000CC112u)
        || x > (int16_t)rd16(m, 0x000CC114u)) {               /* 0x138310 cmp2.w */
        Obj_Free(m, obj);                                     /* 0x138318 -> 0x138382 */
        return 0;
    }
    int16_t y = (int16_t)rd16(m, obj + 0x18u);                /* 0x13831C */
    r->d[0] = setw(r->d[0], (uint16_t)y);
    if (y < (int16_t)rd16(m, 0x000CC116u)
        || y > (int16_t)rd16(m, 0x000CC118u)) {               /* 0x138320 cmp2.w */
        Obj_Free(m, obj);                                     /* 0x138328 -> 0x138382 */
        return 0;
    }

    wr32(m, obj, 0x00138334u);                                /* 0x13832C */
    return 0;
}
