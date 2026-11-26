/* ROM 0x0FB11E: explosion/effect init (A0 = object).
 * Seeds from 0x0F824C (0x0CEDE4), copies +0x10.b into +0x23, sets +0x1D = 0x84,
 * loads sprite/hitbox from 0x0FACC6 via 0x0CFA7C(+0x58), subtracts scroll
 * (A5+0x4224/0x4228), draws sprite (0x001282), and arms tick 0x0FB160. */
#include "fn_auto.h"
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

int Fn_0FB11E(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000F824Cu);                         /* 0x0FB11E..0x0FB122 */
    wr8(m, obj + 0x23u, rd8(m, obj + 0x10u));                /* 0x0FB128 */
    wr8(m, obj + 0x1Du, 0x84u);                              /* 0x0FB12E */
    Obj_0CFA7C(m, obj, 0x000FACC6u, rd16(m, obj + 0x58u));   /* 0x0FB134..0x0FB13C */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u)); /* 0x0FB142..0x0FB146 */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x0FB14A..0x0FB14E */

    draw_sprite_regs(m, r);                                  /* 0x0FB152 -> 0x001282 */
    wr32(m, obj, 0x000FB160u);                               /* 0x0FB158 */
    return 0;
}
