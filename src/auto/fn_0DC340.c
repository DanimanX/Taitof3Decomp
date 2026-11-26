/* ROM 0x0DC340: aimed projectile initializer with sound 0x16 (A0 = object).
 * Plays hooked sound 0x16 (0x13F238 + 0x0039D8), then branches to 0x0DC076:
 * retires via 0x0CEE74 if +0x58.w > 0x167 or +0x60.w > 0x32 (unsigned),
 * otherwise offsets (+0x14, +0x18) by (-0x18, -0x18), seeds from 0x0D8FE2
 * (0x0CEDE4), picks one of 3 behaviours (0x0DC124, 0x0DC1CC, 0x0DC238) via
 * Obj_Next4Way, computes velocity via 0x0D2D44 into (+0x50, +0x54), scrolls Y,
 * draws sprite (0x001282), and arms 0x0DC0F8. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

/* ROM 0x0D2D44: scale (0x0D2D64[angle], 0x0D2ECC[angle]) by speed (0..50). */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)(int32_t)(int16_t)d0w * 4u;
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = (uint32_t)(d0 * (uint32_t)d1w);
    *out_d1 = (uint32_t)(d1 * (uint32_t)d1w);
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

int Fn_0DC340(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Sound_PlayHooked(m, 0x16);                          /* 0x0DC340..0x0DC350 */

    if (rd16(m, obj + 0x58u) > 0x0167u ||               /* 0x0DC076..0x0DC07C */
        rd16(m, obj + 0x60u) > 0x0032u) {               /* 0x0DC082..0x0DC088 */
        Obj_StartDying(m, obj);                         /* 0x000CEE74 */
        return 0;
    }

    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x18u)); /* 0x0DC08E */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x18u)); /* 0x0DC094 */

    Obj_0CEDE4(m, obj, 0x000D8FE2u);                    /* 0x0DC09A..0x0DC09E */

    uint32_t a1 = 0x000DC124u;                          /* 0x0DC0A4 */
    uint16_t rnd = Obj_Next4Way(m);                     /* 0x0DC0A8 */
    if (rnd > 0x33u) {                                  /* 0x0DC0AE..0x0DC0B2 */
        a1 = 0x000DC1CCu;                               /* 0x0DC0B6 */
        if (rnd > 0x99u)                                /* 0x0DC0BA..0x0DC0BE */
            a1 = 0x000DC238u;                           /* 0x0DC0C2 */
    }
    Obj_StartBehaviour(m, obj, a1);                     /* 0x0DC0C6 */

    uint32_t vx, vy;
    sub_0D2D44(m, rd16(m, obj + 0x58u), rd16(m, obj + 0x60u), &vx, &vy); /* 0x0DC0CC..0x0DC0D4 */
    wr32(m, obj + 0x50u, vx);                           /* 0x0DC0DA */
    wr32(m, obj + 0x54u, vy);                           /* 0x0DC0DE */

    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x0DC0E2..0x0DC0E6 */
    draw_sprite_regs(m, obj, r);                        /* 0x0DC0EA -> 0x001282 */
    wr32(m, obj, 0x000DC0F8u);                          /* 0x0DC0F0 */
    return 0;
}
