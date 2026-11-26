/* ROM 0x11257E: horizontal scrolling object tick (A0 = object).
 * Computes screen X (+0x14) = +0x48 - (A5-0x5D98). If X >= 0x140, re-arms
 * 0x11257E. Otherwise animates via +0x44 (0x0CBD52), draws sprite (0x001282),
 * and frees self (0x0105EC) if X < -0x30 or re-arms 0x11257E. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

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
        r->d[1] = (r->d[1] & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(spr_hdr >> 16);
        r->d[0] = 0x0000FFFFu;
        r->d[1] = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_11257E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = rd32(m, obj + 0x48u) - rd32(m, F3_A5 - 0x5D98u); /* 0x11257E..0x112582 */
    r->d[0] = d0;
    wr32(m, obj + 0x14u, d0);                                      /* 0x112586 */

    if ((int16_t)rd16(m, obj + 0x14u) >= 0x0140) {                 /* 0x11258A..0x112590 */
        wr32(m, obj, 0x0011257Eu);                                 /* 0x112576 */
        return 0;
    }

    uint32_t a1 = rd32(m, obj + 0x44u);                            /* 0x112592 */
    r->a[1] = a1;
    Obj_Animate(m, obj, a1);                                       /* 0x112596 (0x0CBD52) */
    draw_sprite_regs(m, r);                                        /* 0x11259C (0x001282) */

    if ((int16_t)rd16(m, obj + 0x14u) < -0x0030)                   /* 0x1125A2..0x1125A8 */
        Obj_Free(m, obj);                                          /* 0x1125AE..0x1125B6 (0x0105EC) */
    else
        wr32(m, obj, 0x0011257Eu);                                 /* 0x1125AC -> 0x112576 */

    return 0;
}
