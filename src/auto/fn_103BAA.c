/* ROM 0x103BAA: boss turret projectile initializer (A0 = object).
 * Initializes combat stats from 0x103110 via 0x0CEDE4, sets +0x21 = 4, flag bits,
 * selects angle (+0x58), anim/frame ptr (+0x0C), hitbox (+0x2C..+0x32), and
 * sprite record from +0x10 index, then enters 0x102FA4 (sets frame via 0x0CFA86,
 * computes velocity via 0x0D0CB0 + 13 and 0x0D2D44, applies scroll Y, draws sprite
 * via 0x001282, and transitions to 0x102FE6). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t spr_hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, SPR_CURSOR);

    if (spr_hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w (via table 0x0D4F26, 0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    *out_d0 = rd32(m, 0x000D2D64u + off) * (uint32_t)d1w;
    *out_d1 = rd32(m, 0x000D2ECCu + off) * (uint32_t)d1w;
}

int Fn_103BAA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    Obj_InitMoverB(m, obj, 0x00103110u);                    /* 0x103BAA..0x103BAE */
    wr8(m, obj + 0x21u, 4u);                                /* 0x103BB4 */
    wr8(m, obj + 0x07u, (uint8_t)(rd8(m, obj + 0x07u) | 0x10u)); /* 0x103BBA */
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u)); /* 0x103BC0 */

    int32_t idx = (int16_t)rd16(m, obj + 0x10u);            /* 0x103BC6 */
    wr16(m, obj + 0x58u, rd16(m, (uint32_t)(0x00103C00 + idx * 2))); /* 0x103BCA */
    wr32(m, obj + 0x0Cu, rd32(m, (uint32_t)(0x00103C0E + idx * 4))); /* 0x103BD4 */

    uint32_t hb = (uint32_t)(0x00103C2A + idx * 8);         /* 0x103BDE */
    wr16(m, obj + 0x2Cu, rd16(m, hb));                      /* 0x103BE6 */
    wr16(m, obj + 0x2Eu, rd16(m, hb + 2u));                 /* 0x103BEA */
    wr16(m, obj + 0x30u, rd16(m, hb + 4u));                 /* 0x103BEE */
    wr16(m, obj + 0x32u, rd16(m, hb + 6u));                 /* 0x103BF2 */

    uint32_t a1 = rd32(m, (uint32_t)(0x00103A6E + idx * 4)); /* 0x103BF6 */

    /* 0x103BFC -> 0x102FA4 */
    Obj_SetFrame(m, obj, a1);                               /* 0x102FA4 -> 0x0CFA86 */
    uint16_t spd = (uint16_t)(rd16(m, 0x000D0D82u + ((rd8(m, F3_A5 + 0x087Fu) >> 3) * 2u)) + 0x000Du); /* 0x102FAA..0x102FB0 */
    wr16(m, obj + 0x60u, spd);                              /* 0x102FB4 */
    d1 = setw(d1, spd);                                     /* 0x102FB8 */
    d0 = setw(d0, rd16(m, obj + 0x58u));                    /* 0x102FBA */
    sub_0D2D44(m, (uint16_t)d0, (uint16_t)d1, &d0, &d1);    /* 0x102FBE */
    wr32(m, obj + 0x50u, d0);                               /* 0x102FC4 */
    wr32(m, obj + 0x54u, d1);                               /* 0x102FC8 */

    d0 = rd32(m, F3_A5 + 0x4228u);                          /* 0x102FCC */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);        /* 0x102FD0 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);        /* 0x102FD4 */
    draw_sprite_regs(m, obj, &d0, &d1, &a1);                /* 0x102FD8 */
    wr32(m, obj, 0x00102FE6u);                              /* 0x102FDE */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
