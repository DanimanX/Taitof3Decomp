/* ROM 0x10204A: scrolling particle/trail emitter tick (A0 = object).
 * Subtracts stage scroll (A5+0x4224, A5+0x4228) from (+0x14, +0x18).
 * Decrements +0x21 and jumps to Obj_StartDying (0x0CEE74) when 0.
 * Otherwise decrements +0x23; when it hits 0 resets +0x23=0x0A, toggles
 * +0x22, and spawns 0x1020B6 (if +0x22 != 0) or 0x102198 (if 0) in group 9. */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

int Fn_10204A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t dx = rd32(m, F3_A5 + 0x4224u);                    /* 0x10204A */
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) - dx);           /* 0x10204E */
    uint32_t dy = rd32(m, F3_A5 + 0x4228u);                    /* 0x102052 */
    r->d[0] = dy;
    wr32(m, obj + 0x18u, rd32(m, obj + 0x18u) - dy);           /* 0x102056 */

    uint8_t life = (uint8_t)(rd8(m, obj + 0x21u) - 1u);        /* 0x10205A */
    wr8(m, obj + 0x21u, life);
    if (life == 0u) {                                          /* 0x10205E */
        Obj_StartDying(m, obj);                                /* 0x0CEE74 */
        return 0;
    }

    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);         /* 0x102064 */
    wr8(m, obj + 0x23u, cnt);
    if (cnt != 0u)                                             /* 0x102068 */
        return 0;

    wr8(m, obj + 0x23u, 0x0Au);                                /* 0x10206C */
    uint8_t tog = (uint8_t)(~rd8(m, obj + 0x22u));             /* 0x102072 */
    wr8(m, obj + 0x22u, tog);
    if (tog != 0u)                                             /* 0x102076 */
        r->d[0] = Obj_Spawn(m, obj, 0x001020B6u, 9u, 2u, 0u);  /* 0x10208C */
    else
        r->d[0] = Obj_Spawn(m, obj, 0x00102198u, 9u, 2u, 0u);  /* 0x1020AA */
    return 0;
}
