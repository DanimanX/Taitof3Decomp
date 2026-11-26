/* ROM 0x192A3C: scrolling boss-part entry tick (A0 = object).
 * Subtracts A5+0x4224 from +0x50, copies (+0x50.w, +0x54.w) to (+0x14.w, +0x18.w),
 * draws sprite (0x001282), and re-arms 0x192A3C while +0x50.w > A5+0x4B94.
 * Otherwise plays Sound_PlayHooked 0x25 and transitions to 0x192A7C. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
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

int Fn_192A3C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);                /* 0x192A3C */

    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) - d0);      /* 0x192A40 */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));           /* 0x192A44 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));           /* 0x192A4A */

    draw_sprite_regs(m, r);                               /* 0x192A50 -> 0x001282 */

    uint16_t x = rd16(m, obj + 0x50u);                    /* 0x192A56 */
    r->d[0] = setw(r->d[0], x);
    if ((int16_t)x > (int16_t)rd16(m, F3_A5 + 0x4B94u)) { /* 0x192A5A..0x192A5E */
        wr32(m, obj, 0x00192A3Cu);                        /* 0x192A34 */
        return 0;
    }

    Sound_PlayHooked(m, 0x0025u);                         /* 0x192A60..0x192A70 */
    wr32(m, obj, 0x00192A7Cu);                            /* 0x192A74 */
    return 0;
}
