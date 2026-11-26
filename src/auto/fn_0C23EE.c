/* ROM 0x0C23EE: shrinking/accelerating particle effect tick (A0 = object).
 * Computes scale = (+0x60 * 3) >> 3 into +0x1F/+0x1E, positions (+0x14, +0x18)
 * from (+0x44, +0x48) + (scale*0x20 << 8, scale*0x18 << 8), draws sprite
 * (0x001282), integrates (+0x44, +0x48) += (+0x4C, +0x50) and accelerates
 * (+0x4C, +0x50) by +0x3C, and decrements +0x60 (re-arming 0x0C23EE if >= 0,
 * else freeing via Obj_Free 0x0105EC). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
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

int Fn_0C23EE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = r->a[1];

    uint32_t sc = ((uint32_t)rd16(m, obj + 0x60u) * 3u) >> 3; /* 0x0C23EE..0x0C23F6 */
    wr8(m, obj + 0x1Fu, (uint8_t)sc);                   /* 0x0C23F8 */
    wr8(m, obj + 0x1Eu, (uint8_t)sc);                   /* 0x0C23FC */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x44u));         /* 0x0C2400 */
    d0 = ((uint32_t)rd8(m, obj + 0x1Fu) * 0x20u) << 8;  /* 0x0C2406..0x0C2410 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);    /* 0x0C2412 */

    wr32(m, obj + 0x18u, rd32(m, obj + 0x48u));         /* 0x0C2416 */
    d0 = ((uint32_t)rd8(m, obj + 0x1Eu) * 0x18u) << 8;  /* 0x0C241C..0x0C2426 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);    /* 0x0C2428 */

    draw_sprite_regs(m, obj, &d0, &d1, &a1);            /* 0x0C242C */

    d0 = rd32(m, obj + 0x4Cu);                          /* 0x0C2432 */
    wr32(m, obj + 0x44u, rd32(m, obj + 0x44u) + d0);    /* 0x0C2436 */
    d0 = rd32(m, obj + 0x50u);                          /* 0x0C243A */
    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + d0);    /* 0x0C243E */

    wr32(m, obj + 0x4Cu, rd32(m, obj + 0x4Cu) + 0x3Cu); /* 0x0C2442 */
    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) + 0x3Cu); /* 0x0C244A */

    int16_t rem = (int16_t)(rd16(m, obj + 0x60u) - 1u); /* 0x0C2452 */
    wr16(m, obj + 0x60u, (uint16_t)rem);
    if (rem >= 0)                                       /* 0x0C2456 */
        wr32(m, obj + 0x00u, 0x000C23EEu);              /* 0x0C23E6 */
    else
        Obj_Free(m, obj);                               /* 0x0C245A */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
