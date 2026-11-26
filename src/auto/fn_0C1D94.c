/* ROM 0x0C1D94: sine-bounced sprite Y tick (A0 = object).
 * Reads (A5+0x8F2).w * 2 & 0xFF, indexes 0x027E40 sin table * 0x80,
 * reflects positive values negative, adds 0x880000 to +0x18, draws sprite
 * (0x001282), and re-arms 0x0C1D94. */
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

int Fn_0C1D94(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t idx = (uint16_t)((rd16(m, F3_A5 + 0x08F2u) * 2u) & 0x00FFu); /* 0x0C1D94..0x0C1DA0 */
    int16_t s = (int16_t)rd16(m, 0x00027E40u + (uint32_t)idx * 2u); /* 0x0C1DA4 */
    int32_t y = (int32_t)s * 0x0080;                         /* 0x0C1DA8 */
    if (y > 0)                                               /* 0x0C1DAC */
        y = -y;                                              /* 0x0C1DB0 */
    y += 0x00880000;                                         /* 0x0C1DB2 */
    wr32(m, obj + 0x18u, (uint32_t)y);                       /* 0x0C1DB8 */

    draw_sprite_regs(m, r);                                  /* 0x0C1DBC -> 0x001282 */
    wr32(m, obj, 0x000C1D94u);                               /* 0x0C1DC2 -> 0x0C1D8C */
    return 0;
}
