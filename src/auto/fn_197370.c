/* ROM 0x197370: boss part follow-parent and draw tick (A0 = object).
 * Updates (+0x50, +0x54) and (+0x14, +0x18) from parent (+0x08) + (+0x16, +0x1A),
 * centers by (+0x10) sprite dimensions * 8, draws sprite (0x001282), and
 * transitions to 0x1973BE once A5+0x4B9E bit 0 (0x80) is set. */
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

int Fn_197370(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                    /* 0x197370 */
    uint16_t x = (uint16_t)(rd16(m, a4 + 0x50u) + rd16(m, obj + 0x16u)); /* 0x197374, 0x19737C */
    uint16_t y = (uint16_t)(rd16(m, a4 + 0x54u) + rd16(m, obj + 0x1Au)); /* 0x197378, 0x197380 */

    wr16(m, obj + 0x50u, x);                               /* 0x197384 */
    wr16(m, obj + 0x54u, y);                               /* 0x197388 */
    wr16(m, obj + 0x14u, x);                               /* 0x19738C */
    wr16(m, obj + 0x18u, y);                               /* 0x197390 */

    uint32_t a1 = rd32(m, obj + 0x10u);                    /* 0x197394 */
    int16_t d1w = (int16_t)(rd16(m, a1) << 3);             /* 0x197398, 0x19739E */
    int16_t d0w = (int16_t)(rd16(m, a1 + 2u) << 3);        /* 0x19739A, 0x19739C */
    wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - d0w)); /* 0x1973A0 */
    wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - d1w)); /* 0x1973A4 */

    r->a[4] = a4;
    draw_sprite_regs(m, r);                                /* 0x1973A8 -> 0x001282 */

    if ((rd8(m, F3_A5 + 0x4B9Eu) & 0x80u) == 0u)           /* 0x1973AE..0x1973B4 */
        wr32(m, obj, 0x00197370u);                         /* 0x197368 */
    else
        wr32(m, obj, 0x001973BEu);                         /* 0x1973B6 */

    return 0;
}
