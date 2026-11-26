/* ROM 0x19EFB8: boss explosion/spawner phase tick (A0 = object, +0x08 = parent).
 * Copies parent (+0x50, +0x54) word coords to self. Every 8 frames (A5+0x878
 * & 7 == 0) spawns 0x19F096 and 0x19F0B2 (group 9, count 2), and every frame
 * spawns 0x19F0CE (group 9, count 2). Counts down +0x06, re-arming 0x19EFB8
 * while > 0, or resetting +0x06 = 0x50 and transitioning to 0x19F046. */
#include "fn_auto.h"
#include "../objects/obj_pool.h"

int Fn_19EFB8(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a4 = rd32(m, obj + 0x08u);                  /* 0x19EFB8 */
    r->a[4] = a4;

    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));           /* 0x19EFBC */
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));           /* 0x19EFC2 */

    if ((rd16(m, F3_A5 + 0x0878u) & 7u) == 0u) {         /* 0x19EFC8..0x19EFD2 */
        (void)Obj_Spawn(m, obj, 0x0019F096u, 9u, 2u, 0u); /* 0x19EFEA */
        (void)Obj_Spawn(m, obj, 0x0019F0B2u, 9u, 2u, 0u); /* 0x19F008 */
    }

    r->d[0] = Obj_Spawn(m, obj, 0x0019F0CEu, 9u, 2u, 0u); /* 0x19F026 */

    int16_t cnt = (int16_t)(rd16(m, obj + 0x06u) - 1u);  /* 0x19F030 */
    wr16(m, obj + 0x06u, (uint16_t)cnt);
    if (cnt > 0) {                                       /* 0x19F034 */
        wr32(m, obj, 0x0019EFB8u);                       /* 0x19EFB0 */
        return 0;
    }

    wr16(m, obj + 0x06u, 0x0050u);                       /* 0x19F038 */
    wr32(m, obj, 0x0019F046u);                           /* 0x19F03E */
    return 0;
}
