/* ROM 0x192B1C: horizontal scrolling boss/effect sprite tick (A0 = object).
 * Advances +0x50 by +6, mirrors +0x50 into A5+0x4B94 when +0x22 == 0, copies
 * (+0x50, +0x54) to (+0x14, +0x18), ticks 0x179A5C animation [+0x40], draws
 * sprite via 0x001282, and re-arms 0x192B1C while +0x50 <= 0x140 else frees
 * via 0x0105EC. */
#include "fn_auto.h"
#include "../objects/obj_anim.h"
#include "../objects/obj_pool.h"
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

int Fn_192B1C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    int16_t x = (int16_t)(rd16(m, obj + 0x50u) + 6u);   /* 0x192B1C */
    wr16(m, obj + 0x50u, (uint16_t)x);

    if (rd8(m, obj + 0x22u) == 0u)                      /* 0x192B20 */
        wr16(m, F3_A5 + 0x4B94u, (uint16_t)x);          /* 0x192B28 */

    wr16(m, obj + 0x14u, (uint16_t)x);                  /* 0x192B2E */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));         /* 0x192B34 */

    uint16_t anim_idx = rd16(m, obj + 0x40u);           /* 0x192B3A */
    if (Obj_AnimTick(m, obj, anim_idx) < 0)             /* 0x192B3E..0x192B48 */
        return -1;

    draw_sprite_regs(m, r);                             /* 0x192B4A */

    if ((int16_t)rd16(m, obj + 0x50u) <= 0x0140) {      /* 0x192B50 */
        wr32(m, obj, 0x00192B1Cu);                      /* 0x192B14 */
        return 0;
    }

    Obj_Free(m, obj);                                   /* 0x192B58..0x192B60 */
    return 0;
}
