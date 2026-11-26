/* ROM 0x10F706: random debris/explosion projectile initializer (A0 = object).
 * Offsets (+0x14, +0x18) by ((Obj_Next4Way & 0xF) - 0x10, -8), seeds stats
 * via Obj_0CEDE4 (0x10C8E2), picks one of 4 behaviour scripts (0x10F7DA[0..3])
 * via Obj_StartBehaviour (0x0D0678), picks random speed 5..12 and angle
 * 0x3A..0x79 via 0x0D2D44 into (+0x50, +0x54), subtracts scroll (A5+0x4224,
 * A5+0x4228) from (+0x14, +0x18), draws sprite (0x001282), and arms 0x10F786. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
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

/* ROM 0x0D2D44: (D0, D1) = {0x0D2D64, 0x0D2ECC}[d0w] * d1w. */
static void sub_0D2D44(const f3mem_t *m, uint16_t d0w, uint16_t d1w,
                       uint32_t *out_d0, uint32_t *out_d1)
{
    uint32_t off = (uint32_t)((int32_t)(int16_t)d0w * 4);
    uint32_t d0 = rd32(m, 0x000D2D64u + off);
    uint32_t d1 = rd32(m, 0x000D2ECCu + off);
    *out_d0 = d0 * (uint32_t)d1w;
    *out_d1 = d1 * (uint32_t)d1w;
}

int Fn_10F706(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint16_t dx = (uint16_t)((Obj_Next4Way(m) & 0x000Fu) - 0x0010u); /* 0x10F706..0x10F710 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx));     /* 0x10F714 */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 8u));     /* 0x10F718 */

    Obj_0CEDE4(m, obj, 0x0010C8E2u);                                 /* 0x10F71C..0x10F720 */

    uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 3u);                 /* 0x10F726..0x10F72C */
    uint32_t beh = rd32(m, 0x0010F7DAu + (uint32_t)idx * 4u);        /* 0x10F730 */
    Obj_StartBehaviour(m, obj, beh);                                 /* 0x10F738 */

    uint16_t d1w = (uint16_t)((Obj_Next4Way(m) & 7u) + 5u);          /* 0x10F73E..0x10F74A */
    uint16_t d0w = (uint16_t)((Obj_Next4Way(m) & 0x3Fu) + 0x003Au);  /* 0x10F74C..0x10F756 */

    sub_0D2D44(m, d0w, d1w, &r->d[0], &r->d[1]);                     /* 0x10F75A */
    wr32(m, obj + 0x50u, r->d[0]);                                   /* 0x10F760 */
    wr32(m, obj + 0x54u, r->d[1]);                                   /* 0x10F764 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u)); /* 0x10F768..0x10F76C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x10F770..0x10F774 */

    draw_sprite_regs(m, r);                                          /* 0x10F778 */

    wr32(m, obj, 0x0010F786u);                                       /* 0x10F77E */
    return 0;
}
