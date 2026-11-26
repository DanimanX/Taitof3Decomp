/* ROM 0x0E1098: linked-list spawner/drawer tick (A0 = head obj, +0x08 = parent,
 * +0x54 = next in doubly-linked list). Clears +0x06 when parent state == 0;
 * when active, counts down +0x23 every 7 frames to spawn 0x0E1134 at list head;
 * walks +0x54 list and draws/scrolls nodes whose +0x06 == 0. Frees self via
 * 0x0CEE74 when list is empty and +0x06 == 0. */
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

int Fn_0E1098(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                       /* 0x0E1098 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x06u) == 0u)                             /* 0x0E109C */
        wr8(m, obj + 0x06u, 0u);                              /* 0x0E10A4 */

    if (rd8(m, obj + 0x06u) != 0u) {                          /* 0x0E10A8 */
        uint8_t b = (uint8_t)(rd8(m, obj + 0x23u) - 1u);      /* 0x0E10B0 */
        wr8(m, obj + 0x23u, b);
        if (b == 0u) {
            uint32_t d0 = Obj_Spawn(m, obj, 0x000E1134u, 9u, 2u, 0u); /* 0x0E10CA */
            r->d[0] = d0;
            if ((int32_t)d0 >= 0) {
                a1 = d0;
                uint32_t a2 = rd32(m, obj + 0x54u);
                r->a[1] = a1;
                r->a[2] = a2;
                wr8(m, a1 + 0x06u, 1u);                       /* 0x0E10E0 */
                wr32(m, obj + 0x54u, a1);                     /* 0x0E10E6 */
                wr32(m, a1 + 0x50u, obj);                     /* 0x0E10EA */
                wr32(m, a1 + 0x54u, a2);                      /* 0x0E10EE */
                if ((int32_t)a2 >= 0)                         /* 0x0E10F2 */
                    wr32(m, a2 + 0x50u, a1);                  /* 0x0E10F6 */
            }
            wr8(m, obj + 0x23u, 7u);                          /* 0x0E10FA */
        }
    }

    uint32_t cur = rd32(m, obj + 0x54u);                      /* 0x0E1100 */
    r->d[0] = cur;
    if ((int32_t)cur < 0) {
        if (rd8(m, obj + 0x06u) == 0u)                        /* 0x0E1108 */
            Obj_StartDying(m, obj);                           /* 0x0E110C -> 0x0CEE74 */
        return 0;
    }

    do {                                                      /* 0x0E1114 */
        r->a[0] = cur;
        if (rd8(m, cur + 0x06u) == 0u) {
            uint32_t d0 = rd32(m, F3_A5 + 0x4228u);
            wr32(m, cur + 0x18u, rd32(m, cur + 0x18u) - d0);
            r->d[0] = d0;
            draw_sprite_regs(m, r);                           /* 0x0E1126 -> 0x001282 */
        }
        cur = rd32(m, cur + 0x54u);
        r->d[0] = cur;
    } while ((int32_t)cur >= 0);

    return 0;
}
