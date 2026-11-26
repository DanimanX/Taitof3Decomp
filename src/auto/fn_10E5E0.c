/* ROM 0x10E5E0: parent-tracked spawner tick (A0 = object).
 * Reads +0x08 parent A1; if A1+0x06 == 0, jumps to Obj_StartDying (0x0CEE74).
 * Otherwise copies parent (+0x3A, +0x3E) + (+0x20, -0x2D) into (+0x14, +0x18),
 * decrements +0x23.b, and when 0 rolls 3 + (Obj_Next4Way & 3) into +0x23.b,
 * toggles +0x22.b, and spawns 0x10E662 (when != 0) or 0x10E734 (when == 0)
 * in group 9 (2 groups, arg 0). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

int Fn_10E5E0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = rd32(m, obj + 0x08u);                       /* 0x10E5E0 */
    r->a[1] = a1;

    if (rd8(m, a1 + 0x06u) == 0u) {                           /* 0x10E5E4..0x10E5E8 */
        Obj_StartDying(m, obj);
        return 0;
    }

    wr32(m, obj + 0x14u, rd32(m, a1 + 0x3Au));                /* 0x10E5EE */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 0x0020u)); /* 0x10E5F4 */
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x3Eu));                /* 0x10E5FA */
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x002Du)); /* 0x10E600 */

    uint8_t cnt = (uint8_t)(rd8(m, obj + 0x23u) - 1u);        /* 0x10E606 */
    wr8(m, obj + 0x23u, cnt);
    if (cnt != 0u)                                            /* 0x10E60A */
        return 0;

    uint16_t wait = (uint16_t)((Obj_Next4Way(m) & 3u) + 3u);  /* 0x10E60E..0x10E618 */
    wr8(m, obj + 0x23u, (uint8_t)wait);                       /* 0x10E61A */

    uint8_t tog = (uint8_t)~rd8(m, obj + 0x22u);              /* 0x10E61E */
    wr8(m, obj + 0x22u, tog);
    uint32_t fn = (tog != 0u) ? 0x0010E662u : 0x0010E734u;    /* 0x10E622..0x10E650 */
    r->d[0] = Obj_Spawn(m, obj, fn, 9u, 2u, 0u);
    return 0;
}
