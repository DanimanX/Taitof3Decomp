/* ROM 0x10E698: scrolling scripted effect tick (A0 = object).
 * Subtracts stage scroll (A5+0x4224, A5+0x4228) from (+0x14, +0x18), subtracts
 * 0x124F8 from +0x14, steps script via 0x0D0690, tail-branches to Obj_StartDying
 * (0x0CEE74) if +0x4C == 0, else draws sprite via 0x001282. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x0D0690 with D0/A1 register tracking. */
static void step_script_regs(const f3mem_t *m, uint32_t obj, uint32_t *d0_io, uint32_t *a1_io)
{
    uint16_t cnt = (uint16_t)(rd16(m, obj + 0x4Cu) - 1u);
    wr16(m, obj + 0x4Cu, cnt);
    if (cnt != 0u)
        return;

    uint32_t a1 = rd32(m, obj + 0x48u) + 0x0Cu;
    wr32(m, obj + 0x48u, a1);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    if ((int16_t)d0 < 0) {
        int16_t back = (int16_t)rd16(m, a1);
        if (back == 0) {
            *d0_io = setw(*d0_io, d0);
            *a1_io = a1;
            return;
        }
        a1 -= (uint32_t)(int32_t)back;
        wr32(m, obj + 0x48u, a1);
        wr16(m, obj + 0x4Cu, rd16(m, a1));
        a1 += 2u;
        uint16_t pal = rd16(m, a1);
        a1 += 2u;
        *d0_io = setw(*d0_io, pal);
        wr8(m, obj + 0x1Cu, (uint8_t)pal);
        wr32(m, obj + 0x10u, rd32(m, a1));
        a1 += 4u;
        wr32(m, obj + 0x68u, rd32(m, a1));
        *a1_io = a1;
        return;
    }

    wr16(m, obj + 0x4Cu, d0);
    uint16_t pal = rd16(m, a1);
    a1 += 2u;
    *d0_io = setw(*d0_io, pal);
    wr8(m, obj + 0x1Cu, (uint8_t)pal);
    wr32(m, obj + 0x10u, rd32(m, a1));
    a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    *a1_io = a1;
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

int Fn_10E698(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u)); /* 0x10E698..0x10E69C */
    uint32_t vy = rd32(m, F3_A5 + 0x4228u);                                /* 0x10E6A0 */
    r->d[0] = vy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - vy);                       /* 0x10E6A4 */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - 0x000124F8u);              /* 0x10E6A8 */

    step_script_regs(m, obj, &r->d[0], &r->a[1]);                          /* 0x10E6B0 -> 0x0D0690 */
    if (rd16(m, obj + 0x4Cu) == 0u) {                                      /* 0x10E6B6..0x10E6BA */
        Obj_StartDying(m, obj);                                            /* 0x0CEE74 */
        return 0;
    }

    draw_sprite_regs(m, r);                                                /* 0x10E6C0 -> 0x001282 */
    return 0;
}
