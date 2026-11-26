/* ROM 0x1918BA: expanding/fading sprite particle tick (A0 = object).
 * Shifts +0x50 by -5, syncs (+0x14, +0x18) from (+0x50, +0x54), decrements +0x1F,
 * adds +0x60.b into zoom +0x1E.b, and advances +0x60 by +0x40. When zoom exceeds
 * 0x100, decrements A5+0x4B92 and frees self; otherwise draws sprite (0x001282)
 * and re-arms 0x1918BA. */
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

int Fn_1918BA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t px = (uint16_t)(rd16(m, obj + 0x50u) - 5u);    /* 0x1918BA */
    wr16(m, obj + 0x50u, px);
    wr16(m, obj + 0x14u, px);                               /* 0x1918C0 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));             /* 0x1918C6 */
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 1u)); /* 0x1918CC */

    uint32_t d0 = rd8(m, obj + 0x60u);                      /* 0x1918D2 bfextu {0:8} */
    uint32_t d1 = rd8(m, obj + 0x1Eu);                      /* 0x1918D8 bfextu {0:8} */
    d0 = setw(d0, (uint16_t)((uint16_t)d0 + (uint16_t)d1)); /* 0x1918DE */

    if ((int16_t)d0 > 0x0100) {                             /* 0x1918E0..0x1918E4 */
        wr16(m, F3_A5 + 0x4B92u, (uint16_t)(rd16(m, F3_A5 + 0x4B92u) - 1u)); /* 0x1918FA */
        Obj_Free(m, obj);                                   /* 0x1918FE..0x191906 */
        r->d[0] = d0;
        r->d[1] = d1;
        return 0;
    }

    wr8(m, obj + 0x1Eu, (uint8_t)d0);                       /* 0x1918E8 */
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) + 0x0040u)); /* 0x1918EC */

    uint32_t a1 = r->a[1];
    draw_sprite_regs(m, obj, &d0, &d1, &a1);                /* 0x1918F2 */
    wr32(m, obj, 0x001918BAu);                              /* 0x1918F8 -> 0x1918B2 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
