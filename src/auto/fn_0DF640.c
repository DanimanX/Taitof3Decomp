/* ROM 0x0DF640: multi-segment tail/chain spawner & drawer (A0 = object).
 * Syncs +0x06 to 0 when parent (+0x08)->+0x06 == 0. While active (+0x06 != 0),
 * decrements +0x23 countdown and on 0 spawns 0x0DF72C (and 0x0DF7EA if
 * parent+0x06 == 5), linking them into the doubly-linked +0x50/+0x54 chain
 * and resetting +0x23 = 7. Then walks the +0x54 chain: if empty (< 0) and
 * +0x06 == 0 calls Obj_StartDying (0x0CEE74); otherwise for each node with
 * +0x06 == 0 subtracts scroll Y (A5+0x4228) from +0x18 and draws (0x001282). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

/* ROM 0x001282 wrapper preserving D0/D1/A1 return values. */
static void draw_sprite_regs(const f3mem_t *m, uint32_t obj,
                             uint32_t *d0_io, uint32_t *d1_io, uint32_t *a1_io)
{
    uint32_t a2 = rd32(m, obj + 0x10u);
    uint32_t hdr = rd32(m, a2);

    Obj_DrawSprite(m, obj);
    *a1_io = rd32(m, F3_A5 - 0x0CA0u);

    if (hdr == 0x00010001u) {
        uint32_t d0 = (((uint32_t)rd16(m, obj + 0x14u) & 0x0FFFu) << 16)
                    | (uint32_t)rd16(m, a2 + 6u);
        uint16_t d1w = (uint16_t)(rd16(m, obj + 0x1Cu) & 0xFF00u);
        d0 = (d0 & 0xFFFF0000u) | (uint16_t)((uint16_t)d0 ^ d1w);
        if (rd8(m, obj + 0x1Du) != 0u)
            d0 = (d0 & 0xFFFFFF00u) | rd8(m, obj + 0x1Du);
        *d0_io = d0;
        *d1_io = setw(*d1_io, d1w);
    } else {
        uint16_t cols = (uint16_t)(hdr >> 16);
        *d0_io = 0x0000FFFFu;
        *d1_io = ((uint32_t)(uint16_t)(cols - 1u) << 16) | 0x0000FFFFu;
    }
}

static void link_segment(const f3mem_t *m, uint32_t obj, uint32_t seg)
{
    uint32_t a2 = rd32(m, obj + 0x54u);                 /* 0x0DF684 */
    wr8(m, seg + 0x06u, 1u);                            /* 0x0DF688 */
    wr32(m, obj + 0x54u, seg);                          /* 0x0DF68E */
    wr32(m, seg + 0x50u, obj);                          /* 0x0DF692 */
    wr32(m, seg + 0x54u, a2);                           /* 0x0DF696 */
    if ((int32_t)a2 >= 0)                               /* 0x0DF69A */
        wr32(m, a2 + 0x50u, seg);                       /* 0x0DF69E */
}

int Fn_0DF640(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];
    uint32_t a1 = rd32(m, obj + 0x08u);                 /* 0x0DF640 */

    if (rd8(m, a1 + 0x06u) == 0u)                       /* 0x0DF644 */
        wr8(m, obj + 0x06u, 0u);                        /* 0x0DF64C */

    if (rd8(m, obj + 0x06u) != 0u) {                    /* 0x0DF650 */
        uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);
        wr8(m, obj + 0x23u, cnt);                       /* 0x0DF658 */
        if (cnt == 0u) {
            d0 = Obj_Spawn(m, obj, 0x000DF72Cu, 9u, 2u, 0u); /* 0x0DF672 */
            if ((int32_t)d0 >= 0) {
                a1 = d0;
                uint32_t prev = rd32(m, obj + 0x54u);
                link_segment(m, obj, a1);
                if ((int32_t)prev >= 0) {
                    a1 = rd32(m, obj + 0x08u);          /* 0x0DF6A2 */
                    if (rd8(m, a1 + 0x06u) == 5u) {     /* 0x0DF6A6 */
                        d0 = Obj_Spawn(m, obj, 0x000DF7EAu, 9u, 2u, 0u); /* 0x0DF6C2 */
                        if ((int32_t)d0 >= 0) {
                            a1 = d0;
                            link_segment(m, obj, a1);
                        }
                    }
                }
            }
            wr8(m, obj + 0x23u, 7u);                    /* 0x0DF6F2 */
        }
    }

    d0 = rd32(m, obj + 0x54u);                          /* 0x0DF6F8 */
    if ((int32_t)d0 < 0) {
        if (rd8(m, obj + 0x06u) == 0u)                  /* 0x0DF700 */
            Obj_StartDying(m, obj);                     /* 0x0DF704 */
    } else {
        do {
            obj = d0;                                   /* 0x0DF70C */
            if (rd8(m, obj + 0x06u) == 0u) {            /* 0x0DF70E */
                d0 = rd32(m, F3_A5 + 0x4228u);          /* 0x0DF716 */
                wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - d0); /* 0x0DF71A */
                draw_sprite_regs(m, obj, &d0, &d1, &a1); /* 0x0DF71E */
            }
            d0 = rd32(m, obj + 0x54u);                  /* 0x0DF724 */
        } while ((int32_t)d0 >= 0);
    }

    r->a[0] = obj;
    r->d[0] = d0;
    r->d[1] = d1;
    r->a[1] = a1;
    return 0;
}
