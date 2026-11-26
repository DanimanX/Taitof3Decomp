/* ROM 0x11221E: scrolling enemy/obstacle tick (A0 = object).
 * Transitions to 0x11225C if X.w (+0x14) < 0xA0. Otherwise subtracts A5+0x422C
 * from +0x14, clears +0x1F and +0x1C, transitions to 0x11225C if HP.w (+0x24)
 * <= 0; else appends to collision lists (0x0CBDA4), blinks hit-flash colour
 * (0x0CBE34), draws sprite (0x001282), and re-arms 0x11221E. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
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

int Fn_11221E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if ((int16_t)rd16(m, obj + 0x14u) < 0x00A0) {         /* 0x11221E..0x112224 */
        wr32(m, obj, 0x0011225Cu);                        /* 0x112254 */
        return 0;
    }

    uint32_t d0 = rd32(m, F3_A5 + 0x422Cu);               /* 0x112228 */
    r->d[0] = d0;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);      /* 0x11222C */
    wr8(m, obj + 0x1Fu, 0u);                              /* 0x112230 */
    wr8(m, obj + 0x1Cu, 0u);                              /* 0x112234 */

    if ((int16_t)rd16(m, obj + 0x24u) <= 0) {             /* 0x112238..0x11223C */
        wr32(m, obj, 0x0011225Cu);                        /* 0x112254 */
        return 0;
    }

    /* 0x112240: jsr 0x000CBDA4 */
    uint32_t p = rd32(m, F3_A5 + 0x08FEu);
    wr32(m, p, obj);
    wr32(m, F3_A5 + 0x08FEu, p + 4u);
    p = rd32(m, F3_A5 + 0x0906u);
    wr32(m, p, obj);
    r->a[1] = p + 4u;
    wr32(m, F3_A5 + 0x0906u, p + 4u);

    Obj_BlinkColour(m, obj);                              /* 0x112246 -> 0x0CBE34 */
    draw_sprite_regs(m, r);                               /* 0x11224C -> 0x001282 */
    wr32(m, obj, 0x0011221Eu);                            /* 0x112252 -> 0x112216 */
    return 0;
}
