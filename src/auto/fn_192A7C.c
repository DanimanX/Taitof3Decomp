/* ROM 0x192A7C: falling jittering effect/debris tick (A0 = object).
 * Subtracts A5+0x4224 from +0x50.l, adds 2 to +0x54.w, copies +0x50/+0x54
 * words to +0x14/+0x18, adds random jitter in [-1..1] from 0x1794A4(2) to
 * +0x14.w, draws sprite (0x001282), and frees self (0x0105EC) once +0x54.w
 * >= 0x178 (else re-arms 0x192A7C). */
#include "fn_auto.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"
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

int Fn_192A7C(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    wr32(m, obj + 0x50u, rd32(m, obj + 0x50u) - rd32(m, F3_A5 + 0x4224u)); /* 0x192A7C..0x192A80 */
    wr16(m, obj + 0x54u, (uint16_t)(rd16(m, obj + 0x54u) + 2u));           /* 0x192A84 */
    wr16(m, obj + 0x14u, rd16(m, obj + 0x50u));                            /* 0x192A88 */
    wr16(m, obj + 0x18u, rd16(m, obj + 0x54u));                            /* 0x192A8E */

    uint16_t jx = Obj_RandRange(m, 2u);                                    /* 0x192A94..0x192A96 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + jx));           /* 0x192A9C */
    draw_sprite_regs(m, r);                                                /* 0x192AA0 (0x001282) */

    if ((int16_t)rd16(m, obj + 0x54u) < 0x0178)                            /* 0x192AA6..0x192AAC */
        wr32(m, obj, 0x00192A7Cu);                                         /* 0x192A74 */
    else
        Obj_Free(m, obj);                                                  /* 0x192AAE..0x192AB6 (0x0105EC) */

    return 0;
}
