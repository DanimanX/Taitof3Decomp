/* ROM 0x1973BE: boss retreat/exit tick (A0 = object).
 * Subtracts 0x30000 from +0x50.l, copies (+0x50, +0x54) -> (+0x14, +0x18),
 * centers by +0x10 dimensions * 8, adds random vertical jitter from
 * 0x1794A4(1) to +0x18 and parent (+0x08)->+0x54, draws sprite (0x001282),
 * and re-arms 0x1973BE while +0x50.w > -0xC8. Once <= -0xC8, frees child +0x0C
 * (0x0105EC), clears +0x24.l, sets +0x06 = 2, and arms 0x19742A. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
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

int Fn_1973BE(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) - 0x00030000u); /* 0x1973BE */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));               /* 0x1973C6 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));               /* 0x1973CC */

    {
        uint32_t a1 = rd32(m, obj + 0x10u);                   /* 0x1973D2 */
        int16_t d1w = (int16_t)(rd16(m, a1) << 3);            /* 0x1973D6, 0x1973DC */
        int16_t d0w = (int16_t)(rd16(m, a1 + 2u) << 3);       /* 0x1973D8, 0x1973DA */
        wr16(m, obj + 0x14u, (uint16_t)((int16_t)rd16(m, obj + 0x14u) - d0w)); /* 0x1973DE */
        wr16(m, obj + 0x18u, (uint16_t)((int16_t)rd16(m, obj + 0x18u) - d1w)); /* 0x1973E2 */
    }

    {
        uint16_t jit = Obj_RandRange(m, 1u);                  /* 0x1973E6..0x1973E8 */
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + jit)); /* 0x1973EE */
        uint32_t a4 = rd32(m, obj + 0x08u);                   /* 0x1973F2 */
        r->a[4] = a4;
        wr16(m, a4 + 0x54u, (uint16_t)(rd16(m, a4 + 0x54u) + jit)); /* 0x1973F6 */
    }

    draw_sprite_regs(m, r);                                   /* 0x1973FA -> 0x001282 */

    if ((int16_t)rd16(m, obj + 0x50u) > -0x00C8) {            /* 0x197400..0x197406 */
        wr32(m, obj + 0x00u, 0x001973BEu);                    /* 0x1973B6 */
        return 0;
    }

    Obj_Free(m, rd32(m, obj + 0x0Cu));                        /* 0x197408..0x197412 */
    wr32(m, obj + 0x24u, 0u);                                 /* 0x197416 */
    wr16(m, obj + 0x06u, 2u);                                 /* 0x19741A (0x197420 beq folded) */
    wr32(m, obj + 0x00u, 0x0019742Au);                        /* 0x197422 */
    return 0;
}
