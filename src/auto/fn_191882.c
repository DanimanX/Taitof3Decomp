/* ROM 0x191882: shrinking sprite particle tick (A0 = object).
 * Shifts +0x50 by -5, syncs (+0x14, +0x18) from (+0x50, +0x54), adjusts +0x1F
 * by -2 and zoom +0x1E by -0x10, draws sprite via 0x001282, and re-arms
 * 0x191882 while +0x1E != 0; otherwise sets +0x60 = 0x400 and arms 0x1918BA. */
#include "fn_auto.h"
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

int Fn_191882(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t px = (uint16_t)(rd16(m, obj + 0x50u) - 5u);        /* 0x191882 */
    wr16(m, obj + 0x50u, px);
    wr16(m, obj + 0x14u, px);                                   /* 0x191888 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                 /* 0x19188E */
    wr8(m, obj + 0x1Fu, (uint8_t)(rd8(m, obj + 0x1Fu) - 2u));   /* 0x191894 */
    wr8(m, obj + 0x1Eu, (uint8_t)(rd8(m, obj + 0x1Eu) - 0x10u));/* 0x19189A */

    draw_sprite_regs(m, obj, &r->d[0], &r->d[1], &r->a[1]);     /* 0x1918A0 */

    if (rd8(m, obj + 0x1Eu) != 0u) {                            /* 0x1918A6..0x1918AA */
        wr32(m, obj, 0x00191882u);                              /* 0x19187A */
        return 0;
    }

    wr16(m, obj + 0x60u, 0x0400u);                              /* 0x1918AC */
    wr32(m, obj, 0x001918BAu);                                  /* 0x1918B2 */
    return 0;
}
