/* ROM 0x0C18AE: falling object tick (A0 = object).
 * Draws sprite (0x001282), integrates Y (+0x18) by +0x44 and accelerates
 * +0x44 by +0x40; when +0x18.w >= 0x80 clears A5+0x8D2 and frees self
 * (0x0105EC), otherwise re-arms 0x0C18AE. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj, f3regs_t *r)
{
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

int Fn_0C18AE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    draw_sprite_regs(m, obj, r);                        /* 0x0C18AE -> 0x001282 */

    uint32_t vy = rd32(m, obj + 0x44u);                 /* 0x0C18B4 */
    r->d[0] = vy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + vy);    /* 0x0C18B8 */
    wr32(m, obj + 0x44u, vy + 0x40u);                   /* 0x0C18BC */

    if ((int16_t)rd16(m, obj + 0x18u) >= 0x80) {        /* 0x0C18C4..0x0C18CA */
        wr32(m, F3_A5 + 0x08D2u, 0u);                   /* 0x0C18D0 */
        Obj_Free(m, obj);                               /* 0x0C18D6 -> 0x0105EC */
        return 0;
    }

    wr32(m, obj, 0x000C18AEu);                          /* 0x0C18A6 */
    return 0;
}
