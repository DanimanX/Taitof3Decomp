/* ROM 0x130A72: homing "laser sight" projectile tick (A0 = object).
 * A bomb-clear flag (A5-0x144B bit 4) frees it. Otherwise 0x0CBD24 steps the
 * animation from the table at +0x52: while the current record still has a
 * duration the object just re-runs 0x130BE4 (which re-derives its screen
 * position from the velocity +0x56/+0x5A and the 0x5E scale, orients and draws
 * the sprite) and re-arms 0x130A72. When the animation runs out the homing
 * velocities are computed from two 0x001D8A draws around the target
 * (A5+0x30E2 +0x14/+0x18), divided by 0x64 + (+0x5E>>1) and the zoom scale
 * 5*(0x64 + +0x5E), stored as words in +0x44/+0x46/+0x48, the base sprite
 * 0x00029820 is installed and the tick moves on to 0x130B16. */
#include "fn_auto.h"
#include "../objects/obj_common.h"     /* Obj_AnimFrameStep (0x0CBD24) */
#include "../objects/obj_pool.h"       /* Obj_Free */
#include "../objects/obj_shared.h"     /* Obj_ExplodeTick (0x0CBB2C) */
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

#define BOUNDS_12EDA4 0x0012EDA4u

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* divu.w/divs.w: 32-bit dividend, 16-bit divisor; quotient low, remainder high
 * (D0 unchanged on overflow or a zero divisor). */
static uint32_t divs_w(uint32_t d, uint16_t div)
{
    int32_t dv = (int32_t)(int16_t)div;
    if (dv == 0)
        return d;
    int32_t q = (int32_t)d / dv;
    int32_t r = (int32_t)d % dv;
    if (q > 32767 || q < -32768)
        return d;
    return ((uint32_t)(uint16_t)r << 16) | (uint16_t)q;
}

/* divsl.l <ea>,D0:D0: plain 32-bit signed divide into D0. */
static uint32_t divsl_32(uint32_t num, int32_t div)
{
    int32_t n = (int32_t)num;
    if (div == 0 || (n == (-2147483647 - 1) && div == -1))
        return num;
    return (uint32_t)(n / div);
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x00649A with the full 32-bit D0 result (D1 is preserved there). */
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

/* ROM 0x0CBB2C with register tracking (see fn_0D63A8.c): reaches three exits,
 * the last of which leaves the 0x001282 sprite registers. */
static void explode_tick_regs(const f3mem_t *m, uint32_t obj,
                              uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t d1_in = *d1_io;
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
    uint16_t ang = Obj_Atan2(m, dx, dy);
    int atan_exg = (((int16_t)dx < 0) != ((int16_t)dy < 0));
    uint32_t ang_hi = atan_exg ? (d1_in & 0xFFFF0000u) : (d0_dist & 0xFFFF0000u);

    Obj_ExplodeTick(m, obj);

    if (is_lt) {
        *d0_io = setw(d0_dist, diff);
        *d1_io = setw(d1_in, dy);
        *a1_io = a1 + 4u;
        return;
    }
    if ((rd8(m, F3_A5 + 0x1D58u) & 0x02u) == 0u) {
        *d0_io = (ang_hi & 0xFFFF0000u) | (uint16_t)(ang << 8);
        *d1_io = setw(d1_in, dy);
        *a1_io = a1 + 4u;
        return;
    }
    *d1_io = rd32(m, obj + 0x14u);
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);
}

/* ROM 0x130BE4: place at (a0+0x56, a0+0x5A)/scale, orient and draw the
 * sprite; near the screen centre it explodes instead (0x0CBB2C), otherwise it
 * joins the hazard list (0x0CBDAE) and the sprite is drawn. D2/D3 are
 * scratch, D0/D1/A1 leave as the last callee leaves them. */
static void sub_130BE4(const f3mem_t *m, uint32_t obj,
                       uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t d0 = (uint32_t)((int32_t)rd32(m, obj + 0x56u) >> 8);   /* 0x130BE4 */
    uint32_t d1 = (uint32_t)((int32_t)rd32(m, obj + 0x5Au) >> 8);   /* 0x130BEA */
    uint16_t d2 = (uint16_t)((uint16_t)(rd16(m, obj + 0x5Eu) >> 1) + 0x0064u);

    d0 = divs_w(d0, d2);                                /* 0x130BFA */
    d1 = divs_w(d1, d2);                                /* 0x130BFC */
    d0 = setw(d0, (uint16_t)(d0 + 0x00A0u));            /* 0x130C00 */
    d1 = setw(d1, (uint16_t)(d1 + 0x0074u));            /* 0x130C04 */
    wr16(m, obj + 0x14u, (uint16_t)d0);                 /* 0x130C08 */
    wr16(m, obj + 0x18u, (uint16_t)d1);                 /* 0x130C0C */
    d0 = divs_w(0x0000639Cu, d2);                       /* 0x130C10..0x130C16 */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 - 0x00FFu));  /* 0x130C18 */
    d0 = setw(d0, (uint16_t)(0u - (uint16_t)d0));       /* 0x130C1C neg.w */
    wr8(m, obj + 0x1Fu, (uint8_t)d0);                   /* 0x130C1E */
    wr8(m, obj + 0x1Eu, (uint8_t)d0);                   /* 0x130C22 */

    /* 0x130C26: cmp2.w (0xE17A,PC,D2.w),D2 -> the bounds pair sits at
     * 0x130C2A + 0xE17A + D2 = 0x12EDA4 + D2 (the {lo,hi} pair is selected by
     * the scale word; the plain form of this idiom, e.g. 0x0CDF20, resolves to
     * the {-0x10,0x140} / {-0x10,0xE8} pairs at 0x0CE1AA / 0x0CE1AE). */
    uint32_t bnd = BOUNDS_12EDA4 + (uint32_t)(int32_t)(int16_t)d2;
    int in = !((int16_t)d2 < (int16_t)rd16(m, bnd)
               || (int16_t)d2 > (int16_t)rd16(m, bnd + 2u));
    if (in && (rd8(m, F3_A5 + 0x1D58u) & 0x02u) != 0u) { /* 0x130C30 btst #1 */
        int16_t x = (int16_t)rd16(m, obj + 0x14u);      /* 0x130C38 / 0x130C40 */
        if (x >= -0x000A && x <= 0x011C) {
            explode_tick_regs(m, obj, d0_io, d1_io, a1_io); /* 0x130C48 jmp 0x0CBB2C */
            return;
        }
    }
    if (in) {                                           /* 0x130C4E */
        uint32_t p = rd32(m, F3_A5 + 0x0906u);
        wr32(m, p, obj);
        wr32(m, F3_A5 + 0x0906u, p + 4u);
    }

    uint32_t a1 = rd32(m, obj + 0x10u);                 /* 0x130C54 */
    d0 = setw(d0, (uint16_t)(d0 - 0x0100u));            /* 0x130C58 subi.w */
    d1 = setw(d1, (uint16_t)d0);                        /* 0x130C5C move.w D0,D1 */
    {
        int32_t p0 = (int32_t)(int16_t)(uint16_t)d0 * (int32_t)(int16_t)rd16(m, a1);
        d0 = ((uint32_t)p0 & 0xFFFF0000u) | (uint16_t)((int16_t)(uint16_t)p0 >> 5);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)d0));
        int32_t p1 = (int32_t)(int16_t)(uint16_t)d1 * (int32_t)(int16_t)rd16(m, a1 + 2u);
        d1 = ((uint32_t)p1 & 0xFFFF0000u) | (uint16_t)((int16_t)(uint16_t)p1 >> 5);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)d1));
    }
    draw_sprite_regs(m, obj, d0_io, d1_io, a1_io);       /* 0x130C6E */
}

int Fn_130A72(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    if ((rd8(m, F3_A5 - 0x144Bu) & 0x10u) != 0u) {      /* 0x130A72 btst #4 */
        Obj_Free(m, obj);                               /* 0x130B52 */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    a1 = rd32(m, obj + 0x52u);                          /* 0x130A7C */
    Obj_AnimFrameStep(m, obj, a1);                      /* 0x130A80 -> 0x0CBD24 */
    if ((int16_t)rd16(m, a1 + (uint32_t)(int16_t)rd16(m, obj + 0x3Au)) >= 0) {
        sub_130BE4(m, obj, &d0, &d1, &a1);              /* 0x130A8E bsr */
        wr32(m, obj + 0x00u, 0x00130A72u);              /* 0x130A92 -> 0x130A6A */
        r->d[0] = d0;
        r->d[1] = d1;
        r->a[1] = a1;
        return 0;
    }

    a1 = rd32(m, F3_A5 + 0x30E2u);                      /* 0x130A94 */
    d1 = setw(d1, (uint16_t)(0x0064u + rd16(m, obj + 0x5Eu))); /* 0x130A98 */
    d1 = (uint32_t)((int32_t)d1 << 5);                  /* 0x130A9E asl.l #5 */
    d0 = Rng_NextMem(m);                                /* 0x130AA0 */
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;      /* 0x130AA6 ext.l */
    d0 <<= 6;                                           /* 0x130AA8 */
    d0 += rd32(m, a1 + 0x14u);                          /* 0x130AAA */
    d0 -= 0x00880000u;                                  /* 0x130AAE */
    d0 <<= 1;                                           /* 0x130AB4 */
    d0 = divsl_32(d0, 5);                               /* 0x130AB6 divsl.l #5 */
    d0 -= rd32(m, obj + 0x56u);                         /* 0x130ABE */
    d0 = divsl_32(d0, (int32_t)d1);                     /* 0x130AC2 */
    wr16(m, obj + 0x44u, (uint16_t)d0);                 /* 0x130AC6 */

    d0 = Rng_NextMem(m);                                /* 0x130ACA */
    d0 = (uint32_t)(int32_t)(int16_t)(uint16_t)d0;      /* 0x130AD0 ext.l */
    d0 <<= 6;                                           /* 0x130AD2 */
    d0 += rd32(m, a1 + 0x18u);                           /* 0x130AD4 */
    d0 -= 0x00640000u;                                  /* 0x130AD8 */
    d0 <<= 1;                                           /* 0x130ADE */
    d0 = divsl_32(d0, 5);                               /* 0x130AE0 */
    d0 -= rd32(m, obj + 0x5Au);                         /* 0x130AE8 */
    d0 = divsl_32(d0, (int32_t)d1);                     /* 0x130AEC */
    wr16(m, obj + 0x46u, (uint16_t)d0);                 /* 0x130AF0 */

    d0 = 0u - rd32(m, obj + 0x5Eu);                     /* 0x130AF4 moveq #0 ; sub.l */
    d0 = divsl_32(d0, (int32_t)d1);                     /* 0x130AFA */
    wr16(m, obj + 0x48u, (uint16_t)d0);                 /* 0x130AFE */

    wr32(m, obj + 0x10u, 0x00029820u);                  /* 0x130B02 */
    wr8(m, obj + 0x1Du, 0u);                            /* 0x130B0A sf */
    wr32(m, obj + 0x00u, 0x00130B16u);                  /* 0x130B0E */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
