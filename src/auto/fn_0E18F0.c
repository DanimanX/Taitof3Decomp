/* ROM 0x0E18F0: trail/chain spawner & drawer (A0 = object).
 * Syncs active flag (+0x06) with parent (+0x08)->+0x06; when active, decrements
 * timer (+0x23) and every 7 frames spawns 0x0E198C (group 9, count 2) linked
 * into the doubly-linked +0x50/+0x54 list. Then walks the +0x54 list, scrolling
 * and drawing (0x001282) nodes with +0x06 == 0, or dying (0x0CEE74) if list is
 * empty and +0x06 == 0. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../video/sprite.h"

int Fn_0E18F0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 0x08u);                   /* 0x0E18F0 */
    r->a[1] = parent;
    if (rd8(m, parent + 0x06u) == 0u)                         /* 0x0E18F4 */
        wr8(m, obj + 0x06u, 0u);                              /* 0x0E18FC */

    if (rd8(m, obj + 0x06u) != 0u) {                          /* 0x0E1900 */
        uint8_t t = (uint8_t)(rd8(m, obj + 0x23u) - 1u);      /* 0x0E1908 */
        wr8(m, obj + 0x23u, t);
        if (t == 0u) {
            uint32_t child = Obj_Spawn(m, obj, 0x000E198Cu, 9u, 2u, 0u); /* 0x0E1910..0x0E1922 */
            r->d[0] = child;
            if ((int32_t)child >= 0) {                        /* 0x0E192C */
                uint32_t next = rd32(m, obj + 0x54u);         /* 0x0E1934 */
                r->a[1] = child;                              /* 0x0E1932 */
                wr8(m, child + 0x06u, 1u);                    /* 0x0E1938 */
                wr32(m, obj + 0x54u, child);                  /* 0x0E193E */
                wr32(m, child + 0x50u, obj);                  /* 0x0E1942 */
                wr32(m, child + 0x54u, next);                 /* 0x0E1946 */
                if ((int32_t)next >= 0)                       /* 0x0E194A */
                    wr32(m, next + 0x50u, child);             /* 0x0E194E */
            }
            wr8(m, obj + 0x23u, 0x07u);                       /* 0x0E1952 */
        }
    }

    /* 0x0E1958: walk +0x54 list */
    {
        uint32_t d0 = rd32(m, obj + 0x54u);
        r->d[0] = d0;
        if ((int32_t)d0 < 0) {
            if (rd8(m, obj + 0x06u) == 0u)                    /* 0x0E1960 */
                Obj_StartDying(m, obj);                       /* 0x0E1964 -> 0x0CEE74 */
            return 0;
        }
        do {
            obj = d0;                                         /* 0x0E196C */
            r->a[0] = obj;
            if (rd8(m, obj + 0x06u) == 0u) {                  /* 0x0E196E */
                uint32_t sy = rd32(m, F3_A5 + 0x4228u);       /* 0x0E1976 */
                wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - sy); /* 0x0E197A */
                Obj_DrawSprite(m, obj);                       /* 0x0E197E -> 0x001282 */
                r->a[1] = rd32(m, F3_A5 - 0x0CA0u);
                r->d[1] = 0x0001FFFFu;
            }
            d0 = rd32(m, obj + 0x54u);                        /* 0x0E1984 */
            r->d[0] = d0;
        } while ((int32_t)d0 >= 0);
    }

    return 0;
}
