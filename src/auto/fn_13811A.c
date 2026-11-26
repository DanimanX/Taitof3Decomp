/* ROM 0x13811A: rotating/orbiting segmented boss/enemy part tick (A0 = object).
 * Copies parent (+0x08)->+0x59 and frees self (0x0105EC) if non-zero.
 * When HP (+0x24) <= 0, awards +0x4C score via 0x0CBDDE, spawns 0x13E4EE
 * explosion, plays sound 0x40 (Sound_PlayHooked), clears (+0x44)->+0x48 if
 * non-null, sets +0x06=10, and transitions to 0x1381E0.
 * While bitfield A5+0x30EE{22:(+0x22+1)} is clear, tracks (+0x0C)->+0x20
 * angle, mirrors (+0x44) position, moves along both angles (0x0CBE12),
 * updates sprite/flash/bounds/collision via 0x138390, and re-arms 0x13811A.
 * Once the bitfield triggers, plays sound 0x3A, clears +0x22/+0x06/+0x42 and
 * (+0x44)->+0x48, checks bomb-clear/HP/explode, spins up +0x06 up to 0x800
 * aiming at player (0x0CC3C6), fires 0x0CCD04 pairs periodically (0x0CC2BC),
 * frees if outside [-0x20..0x140] x [-0x20..0xE8], else arms 0x138334. */
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

static uint32_t swap32(uint32_t v)
{
    return (v << 16) | (v >> 16);
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

/* ROM 0x0CC2BC: D0 = (quot << 16) | rem, D1 = swapped scaled period (or 1). */
static void sub_0CC2BC(const f3mem_t *m, uint16_t d0w,
                       uint32_t *d0_out, uint32_t *d1_out)
{
    uint16_t d1w = (uint16_t)(0x0180u - rd16(m, F3_A5 + 0x087Eu));
    uint32_t d1 = (uint32_t)d1w * (uint32_t)d0w;
    d1 <<= 8;
    d1 = swap32(d1);
    if ((uint16_t)d1 == 0u)
        d1 = 1u;
    uint32_t num = rd16(m, F3_A5 + 0x0878u);
    uint16_t div = (uint16_t)d1;
    uint16_t q = (uint16_t)(num / div);
    uint16_t rem = (uint16_t)(num % div);
    *d0_out = ((uint32_t)q << 16) | rem;
    *d1_out = d1;
}

/* ROM 0x138390: select sprite/palette from +0x20 angle, update flash & draw. */
static void sub_138390(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t ang = (uint16_t)(rd16(m, obj + 0x20u) + 0x0400u); /* 0x138394..0x138398 */
    uint32_t d1 = (uint32_t)((ang >> 14) & 0x03u);          /* 0x13839C */
    uint32_t d0 = (uint32_t)((ang >> 11) & 0x0Fu);          /* 0x1383A0 */

    wr32(m, obj + 0x10u, rd32(m, 0x00138412u + d0 * 4u));   /* 0x1383A4 */
    wr8(m, obj + 0x1Cu, rd8(m, 0x000CC10Eu + d1));          /* 0x1383AA..0x1383B0 */
    r->a[1] = 0x000CC10Eu;
    r->d[0] = d0;
    r->d[1] = d1;

    Obj_11C32C(m, obj);                                     /* 0x1383B6 */

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x0010u)); /* 0x1383BC */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0010u)); /* 0x1383C2 */

    int16_t x = (int16_t)rd16(m, obj + 0x14u);
    int16_t y = (int16_t)rd16(m, obj + 0x18u);
    if (x > -0x0020 && y > -0x0020 && x < 0x0140 && y < 0x00E8) { /* 0x1383C8..0x1383EC */
        draw_sprite_regs(m, r);                             /* 0x1383F0 */
        if (rd8(m, F3_A5 + 0x30F3u) == 0u)                  /* 0x1383F6 */
            sub_0CBDA4(m, obj, &r->a[1]);                   /* 0x1383FE */
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0010u)); /* 0x138404 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 0x0010u)); /* 0x13840A */
}

int Fn_13811A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                 /* 0x13811A */
    r->a[1] = parent;
    uint8_t flag = rd8(m, parent + 0x59u);                  /* 0x13811E */
    wr8(m, obj + 0x59u, flag);
    if (flag != 0u) {                                       /* 0x138124 */
        Obj_Free(m, obj);                                   /* 0x138200 */
        return 0;
    }

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* 0x138128 */
        uint32_t recv = Obj_0CBDDE(m, obj);                 /* 0x138180 */
        r->a[1] = recv;
        uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);      /* 0x138186..0x138188 */
        r->d[0] = pts;
        wr32(m, recv + 0x48u, rd32(m, recv + 0x48u) + pts); /* 0x13818C */
        r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u); /* 0x138190..0x1381AA */
        Sound_PlayHooked(m, 0x0040u);                       /* 0x1381AE..0x1381BE */
        uint32_t link = rd32(m, obj + 0x44u);               /* 0x1381C2 */
        r->d[0] = link;
        if (link != 0u) {
            r->a[1] = link;
            wr32(m, link + 0x48u, 0u);                      /* 0x1381CC */
        }
        wr16(m, obj + 0x06u, 0x000Au);                      /* 0x1381D0 */
        wr32(m, obj + 0x00u, 0x001381E0u);                  /* 0x1381D8 */
        return 0;
    }

    {
        uint16_t d0w = (uint16_t)(rd16(m, obj + 0x22u) + 1u); /* 0x138130..0x138134 */
        r->d[0] = setw(r->d[0], d0w);
        int width = (int)(d0w & 31u);
        if (width == 0)
            width = 32;
        int nz = 0;
        for (int i = 0; i < width; i++) {                   /* 0x138136: bftst (0x30ee,A5){22:D0} */
            uint32_t bit = 22u + (uint32_t)i;
            if ((rd8(m, F3_A5 + 0x30EEu + (bit >> 3)) & (0x80u >> (bit & 7u))) != 0u) {
                nz = 1;
                break;
            }
        }
        if (nz)
            goto detached_tick;                             /* 0x13813C */
    }

    {
        uint32_t target = rd32(m, obj + 0x0Cu);             /* 0x138140 */
        int16_t diff = (int16_t)(rd16(m, target + 0x20u) - rd16(m, obj + 0x20u)); /* 0x138144..0x138148 */
        diff = (int16_t)(diff >> 3);                        /* 0x13814C */
        wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + (uint16_t)diff)); /* 0x13814E */

        uint32_t anchor = rd32(m, obj + 0x44u);             /* 0x138152 */
        r->a[1] = anchor;
        wr32(m, obj + 0x14u, rd32(m, anchor + 0x14u));      /* 0x138156 */
        wr32(m, obj + 0x18u, rd32(m, anchor + 0x18u));      /* 0x13815C */

        uint16_t a_ang = (uint16_t)(rd16(m, anchor + 0x20u) >> 8); /* 0x138162..0x138166 */
        r->d[0] = setw(r->d[0], a_ang);
        move_by_angle_regs(m, r, a_ang);                    /* 0x138168 */

        uint16_t o_ang = (uint16_t)(rd16(m, obj + 0x20u) >> 8); /* 0x13816E..0x138172 */
        r->d[0] = setw(r->d[0], o_ang);
        move_by_angle_regs(m, r, o_ang);                    /* 0x138174 */

        sub_138390(m, r);                                   /* 0x13817A */
        wr32(m, obj + 0x00u, 0x0013811Au);                  /* 0x13817E -> 0x138112 */
        return 0;
    }

detached_tick:
    Sound_PlayHooked(m, 0x003Au);                           /* 0x13820E..0x13821E */
    wr16(m, obj + 0x22u, 0u);                               /* 0x138222 */
    wr16(m, obj + 0x06u, 0u);                               /* 0x138226 */
    wr16(m, obj + 0x42u, 0u);                               /* 0x13822A */
    {
        uint32_t anchor = rd32(m, obj + 0x44u);             /* 0x13822E */
        r->d[0] = anchor;
        if (anchor != 0u) {
            r->a[1] = anchor;
            wr32(m, anchor + 0x48u, 0u);                    /* 0x138238 */
        }
    }

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u)            /* 0x13823C */
        goto detached_explode;

    if ((int32_t)rd32(m, obj + 0x24u) <= 0) {               /* 0x138246 */
        uint32_t recv = Obj_0CBDDE(m, obj);                 /* 0x138338 */
        r->a[1] = recv;
        uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);
        r->d[0] = pts;
        wr32(m, recv + 0x48u, rd32(m, recv + 0x48u) + pts); /* 0x138344 */
        goto detached_explode;
    }

    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) {          /* 0x13824E */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);
        if (x >= -0x000A && x <= 0x011C) {                  /* 0x138256..0x138264 */
            wr8(m, obj + 0x1Du, rd8(m, obj + 0x4Fu));       /* 0x138266 */
            uint32_t a1_exp = rd32(m, F3_A5 + 0x1D50u);     /* 0x0CBD12 */
            r->a[1] = a1_exp;
            uint32_t pts = (uint32_t)rd16(m, obj + 0x4Cu);
            r->d[0] = pts;
            wr32(m, a1_exp + 0x48u, rd32(m, a1_exp + 0x48u) + pts);
            explode_tick_regs(m, r);                        /* 0x0CBD20 -> 0x0CBB2C */
            return 0;
        }
    }

    wr16(m, obj + 0x06u, (uint16_t)(rd16(m, obj + 0x06u) + 0x0010u)); /* 0x138272 */
    if ((int16_t)rd16(m, obj + 0x06u) > 0x0800) {            /* 0x138278 */
        wr16(m, obj + 0x06u, 0x0800u);                      /* 0x138282 */
        wr16(m, obj + 0x42u, (uint16_t)(rd16(m, obj + 0x42u) + 1u)); /* 0x138288 */
        uint8_t b22 = rd8(m, obj + 0x22u);                  /* 0x13828C */
        wr8(m, obj + 0x22u, (uint8_t)(b22 | 0x01u));
        if ((b22 & 0x01u) == 0u) {
            uint32_t tgt = rd32(m, F3_A5 + 0x1D28u);        /* 0x138296 */
            r->a[1] = tgt;
            uint16_t ang = Obj_0CC3C6(m, obj, tgt);         /* 0x13829A */
            wr8(m, obj + 0x23u, (uint8_t)ang);              /* 0x1382A0 */
        }
    }

    {
        uint32_t ang = rd8(m, obj + 0x23u);                 /* 0x1382A4..0x1382A6 */
        r->d[0] = ang;
        move_by_angle_regs(m, r, (uint16_t)ang);            /* 0x1382AA */
    }
    wr16(m, obj + 0x20u, (uint16_t)(rd16(m, obj + 0x20u) + rd16(m, obj + 0x06u))); /* 0x1382B0..0x1382B4 */
    sub_138390(m, r);                                       /* 0x1382B8 */

    sub_0CC2BC(m, 0x0014u, &r->d[0], &r->d[1]);             /* 0x1382BC..0x1382C0 */
    if ((uint16_t)r->d[0] == 0u) {                          /* 0x1382C6 */
        uint32_t d1 = (uint32_t)rd16(m, obj + 0x20u) & 0x0000FF00u; /* 0x1382CC..0x1382D2 */
        r->d[0] = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1); /* 0x1382D4..0x1382EA */
        d1 = (d1 & 0xFFFF0000u) | (uint16_t)((uint16_t)d1 - 0x8000u); /* 0x1382EE */
        r->d[1] = d1;
        r->d[0] = Obj_Spawn(m, obj, 0x000CCD04u, 9u, 2u, d1); /* 0x1382F2..0x138308 */
    }

    {
        int16_t x = (int16_t)rd16(m, obj + 0x14u);          /* 0x13830C */
        r->d[0] = setw(r->d[0], (uint16_t)x);
        if (x < (int16_t)rd16(m, 0x000CC112u) || x > (int16_t)rd16(m, 0x000CC114u)) { /* 0x138310 */
            Obj_Free(m, obj);                               /* 0x138382 */
            return 0;
        }
        int16_t y = (int16_t)rd16(m, obj + 0x18u);          /* 0x13831C */
        r->d[0] = setw(r->d[0], (uint16_t)y);
        if (y < (int16_t)rd16(m, 0x000CC116u) || y > (int16_t)rd16(m, 0x000CC118u)) { /* 0x138320 */
            Obj_Free(m, obj);                               /* 0x138382 */
            return 0;
        }
    }

    wr32(m, obj + 0x00u, 0x00138334u);                      /* 0x13832C */
    return 0;

detached_explode:
    r->d[0] = Obj_Spawn(m, obj, 0x0013E4EEu, 5u, 2u, 2u);   /* 0x138348..0x138362 */
    Sound_PlayHooked(m, 0x0040u);                           /* 0x138366..0x138376 */
    wr32(m, obj + 0x00u, 0x00138382u);                      /* 0x13837A */
    return 0;
}
