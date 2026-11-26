/* ROM 0x0C1D30: dual-sprite overlay tick (A0 = object).
 * Draws sprite 0x053C84 then sprite 0x0537D0 via 0x001282, and re-arms
 * 0x0C1D30. */
#include "fn_auto.h"
#include "../video/sprite.h"

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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0C1D30(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x10u, 0x00053C84u);                       /* 0x0C1D30 */
    draw_sprite_regs(m, r);                                  /* 0x0C1D38 -> 0x001282 */
    wr32(m, obj + 0x10u, 0x000537D0u);                       /* 0x0C1D3E */
    draw_sprite_regs(m, r);                                  /* 0x0C1D46 -> 0x001282 */
    wr32(m, obj, 0x000C1D30u);                               /* 0x0C1D4C -> 0x0C1D28 */
    return 0;
}
