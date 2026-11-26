/* ROM 0x112136: zooming/parabolic approach sprite tick (A0 = object).
 * Advances base X (+0x48) by 0x12000, computes quadratic zoom factor
 * z = 0xA0 - ((0xFF - (+0x44))^2 / 0x19A) into +0x1F and +0x1E, offsets
 * (+0x14, +0x18) from (+0x48, +0x4C) by (z*0x48 << 8, z*0x58 << 8), clears
 * +0x1C, draws sprite (0x001282), decrements +0x44, and re-arms 0x112136
 * while >= 0 else transitions to 0x1121A0. */
#include "fn_auto.h"
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

int Fn_112136(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x48u, rd32(m, obj + 0x48u) + 0x00012000u); /* 0x112136 */

    uint16_t d0w = (uint16_t)(-(int16_t)(uint16_t)(rd16(m, obj + 0x44u) - 0x00FFu)); /* 0x11213E..0x112146 */
    uint32_t d0 = (uint32_t)d0w * (uint32_t)d0w;               /* 0x112148 */
    uint16_t q = (uint16_t)(d0 / 0x019Au);                     /* 0x11214A */
    uint8_t z = (uint8_t)(-(int16_t)(uint16_t)(q - 0x00A0u));  /* 0x11214E..0x112152 */
    wr8(m, obj + 0x1Fu, z);                                    /* 0x112154 */
    wr8(m, obj + 0x1Eu, z);                                    /* 0x112158 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x48u) + (((uint32_t)z * 0x48u) << 8)); /* 0x11215C..0x11216E */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x4Cu) + (((uint32_t)z * 0x58u) << 8)); /* 0x112172..0x112184 */
    wr8(m, obj + 0x1Cu, 0u);                                   /* 0x112188 */

    draw_sprite_regs(m, r);                                    /* 0x11218C */

    int16_t t = (int16_t)(rd16(m, obj + 0x44u) - 1u);          /* 0x112192 */
    wr16(m, obj + 0x44u, (uint16_t)t);
    if (t >= 0)                                                /* 0x112196 */
        wr32(m, obj + 0x00u, 0x00112136u);                     /* 0x11212E */
    else
        wr32(m, obj + 0x00u, 0x001121A0u);                     /* 0x112198 */
    return 0;
}
