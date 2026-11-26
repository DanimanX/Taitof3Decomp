/* ROM 0x10E818: boss/enemy sub-spawner tick (A0 = object, A0+0x08 = parent).
 * Calls Obj_StartDying (0x0CEE74) if parent +0x06.b == 0. Otherwise rolls
 * Obj_Next4Way (0x0D0E3A); if <= 0x5A, spawns 0x10E872 in groups 5..6 and
 * initializes child (+0x14, +0x18) from parent + (0x30, 0x10). */
#include "fn_auto.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared.h"

int Fn_10E818(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x10E818 */
    r->a[4] = a4;

    if (rd8(m, a4 + 0x06u) == 0u) {                      /* 0x10E81C..0x10E820 */
        Obj_StartDying(m, obj);                          /* 0x0CEE74 */
        return 0;
    }

    uint16_t roll = Obj_Next4Way(m);                     /* 0x10E826 -> 0x0D0E3A */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | roll;
    if (roll > 0x005Au)                                  /* 0x10E82C..0x10E830 bhi */
        return 0;

    uint32_t child = Obj_Spawn(m, obj, 0x0010E872u, 5u, 2u, 0u); /* 0x10E834..0x10E84C */
    r->d[0] = child;
    if ((int32_t)child < 0)                              /* 0x10E850..0x10E852 bmi */
        return 0;

    r->a[1] = child;                                     /* 0x10E856 */
    wr32(m, child + 0x14u, rd32(m, a4 + 0x14u));         /* 0x10E858 */
    wr16(m, child + 0x14u, (uint16_t)(rd16(m, child + 0x14u) + 0x0030u)); /* 0x10E85E */
    wr32(m, child + 0x18u, rd32(m, a4 + 0x18u));         /* 0x10E864 */
    wr16(m, child + 0x18u, (uint16_t)(rd16(m, child + 0x18u) + 0x0010u)); /* 0x10E86A */
    return 0;
}
