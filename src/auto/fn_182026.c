/* ROM 0x182026: growing/zooming boss effect sprite tick (A0 = object).
 * Steps Obj_Script10 (0x178982), copies +0x50/+0x54 to +0x14/+0x18, ramps zoom
 * bytes +0x1F and +0x1E by +0x0C up to 0xF8, adjusts X (+0x1F>>3 when +0x1C!=1)
 * and Y (sprite_h*16 * +0x1E >> 9), centers by sprite {w,h}*8, draws via
 * 0x001282, decrements +0x64 and re-arms 0x182026 while > 0 else frees via
 * 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_script10.h"
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

int Fn_182026(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    if (Obj_Script10(m, obj) < 0)                       /* 0x182026 */
        return -1;

    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));         /* 0x18202C */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));         /* 0x182032 */

    {
        uint16_t zx = (uint16_t)(rd8(m, obj + 0x1Fu) + 0x000Cu); /* 0x182038 */
        if ((int16_t)zx > 0x00F8)
            zx = 0x00F8u;
        wr8(m, obj + 0x1Fu, (uint8_t)zx);
    }

    {
        uint16_t zy = (uint16_t)(rd8(m, obj + 0x1Eu) + 0x000Cu); /* 0x182052 */
        if ((int16_t)zy > 0x00F8)
            zy = 0x00F8u;
        wr8(m, obj + 0x1Eu, (uint8_t)zy);
    }

    if (rd8(m, obj + 0x1Cu) != 1u) {                    /* 0x18206C */
        uint16_t off_x = (uint16_t)(rd8(m, obj + 0x1Fu) >> 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + off_x));
    }

    {
        uint32_t spr = rd32(m, obj + 0x10u);            /* 0x182088 */
        uint16_t h16 = (uint16_t)(rd16(m, spr) << 4);
        uint32_t prod = (uint32_t)h16 * (uint32_t)rd8(m, obj + 0x1Eu);
        uint16_t off_y = (uint16_t)((uint16_t)prod >> 9);
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + off_y));
    }

    {
        uint32_t spr = rd32(m, obj + 0x10u);            /* 0x1820A0 */
        uint16_t dy = (uint16_t)(rd16(m, spr) << 3);
        uint16_t dx = (uint16_t)(rd16(m, spr + 2u) << 3);
        wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - dx));
        wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - dy));
    }

    draw_sprite_regs(m, r);                             /* 0x1820B4 */

    {
        uint16_t c64 = (uint16_t)(rd16(m, obj + 0x64u) - 1u); /* 0x1820BA */
        wr16(m, obj + 0x64u, c64);
        if ((int16_t)c64 > 0) {
            wr32(m, obj + 0x00u, 0x00182026u);          /* 0x18201E */
            return 0;
        }
    }

    Obj_Free(m, obj);                                   /* 0x1820C2 */
    return 0;
}
