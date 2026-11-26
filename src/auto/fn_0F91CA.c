/* ROM 0x0F91CA: falling/parabolic debris particle tick (A0 = object).
 * Scrolls +0x14/+0x18/+0x3E by (A5+0x4224, A5+0x4228), integrates Y velocity
 * (+0x54) into +0x18 with gravity +0x36B0, and retires via 0x0CEE74 once
 * +0x18.w > +0x3E.w; otherwise steps script (0x0D0690) and draws (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

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

int Fn_0F91CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, F3_A5 + 0x4224u);              /* 0x0F91CA */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - d0);    /* 0x0F91CE */
    d0 = rd32(m, F3_A5 + 0x4228u);                      /* 0x0F91D2 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0);    /* 0x0F91D6 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x3Eu) - d0);    /* 0x0F91DA */

    d0 = rd32(m, obj + 0x54u);                          /* 0x0F91DE */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) + d0);    /* 0x0F91E2 */
    wr32(m, obj + 0x54u, rd32(m, obj + 0x54u) + 0x36B0u); /* 0x0F91E6 */

    uint16_t y = rd16(m, obj + 0x18u);                  /* 0x0F91EE */
    d0 = setw(d0, y);
    if ((int16_t)y > (int16_t)rd16(m, obj + 0x3Eu)) {    /* 0x0F91F2..0x0F91F6 */
        Obj_StartDying(m, obj);                         /* 0x000CEE74 */
        r->d[0] = d0;
        return 0;
    }

    Obj_StepScript(m, obj);                             /* 0x0F91FC -> 0x0D0690 */
    draw_sprite_regs(m, obj, r);                        /* 0x0F9202 -> 0x001282 */
    return 0;
}
