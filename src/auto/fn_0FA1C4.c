/* ROM 0x0FA1C4: sub-part/turret initializer (A0 = object).
 * Seeds HP/hitboxes from 0x0FA3AC (0x0CEDE4), copies +0x10 byte to +0x20,
 * selects an 8-byte entry from 0x0FA38C using (+0x20)*2 + (parent->+0x06==2)
 * to set +0x21, +0x42, and behaviour (0x0D0678), sets +0x23=8 and +0x34
 * flags, applies scroll (A5+0x4224/0x4228), draws sprite (0x001282), and
 * arms 0x0FA236. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"
#include "../video/sprite.h"

static void draw_sprite_post_regs(const f3mem_t *m, uint32_t obj,
                                  uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_out)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);
    *a1_out = rd32(m, F3_A5 - 0x0CA0u);
    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = (*d1_io & 0xFFFF0000u) | d1w;
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

int Fn_0FA1C4(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    Obj_0CEDE4(m, obj, 0x000FA3ACu);                          /* 0x0FA1C4..0x0FA1C8 */
    wr8(m, obj + 0x20u, rd8(m, obj + 0x10u));                 /* 0x0FA1CE */

    {
        uint16_t idx = (uint16_t)((uint16_t)rd8(m, obj + 0x20u) << 1);
        uint32_t parent = rd32(m, obj + 0x08u);               /* 0x0FA1DC */
        if (rd8(m, parent + 0x06u) == 2u)                     /* 0x0FA1E0 */
            idx++;
        uint32_t rec = 0x000FA38Cu + (uint32_t)idx * 8u;      /* 0x0FA1EC */
        wr8(m, obj + 0x21u, (uint8_t)rd16(m, rec));           /* 0x0FA1F4..0x0FA1F6 */
        wr16(m, obj + 0x42u, rd16(m, rec + 2u));              /* 0x0FA1FA */
        Obj_StartBehaviour(m, obj, rd32(m, rec + 4u));        /* 0x0FA1FE..0x0FA200 */
    }

    wr8(m, obj + 0x23u, 0x08u);                               /* 0x0FA206 */
    wr8(m, obj + 0x34u, (uint8_t)((rd8(m, obj + 0x34u) & ~0x02u) | 0x04u)); /* 0x0FA20C..0x0FA212 */

    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - rd32(m, F3_A5 + 0x4224u)); /* 0x0FA218..0x0FA21C */
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - rd32(m, F3_A5 + 0x4228u)); /* 0x0FA220..0x0FA224 */

    Obj_DrawSprite(m, obj);                                   /* 0x0FA228 -> 0x001282 */
    draw_sprite_post_regs(m, obj, &r->d[0], &r->d[1], &r->a[1]);
    wr32(m, obj, 0x000FA236u);                                /* 0x0FA22E */
    return 0;
}
