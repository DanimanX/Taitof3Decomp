/* ROM 0x1984C0: burst-spawn 8x 0x17A1DE particles + sound 0x3A (A0 = object).
 * Copies parent (+0x08)->+0x50/+0x54 to +0x50/+0x54, loops D2 = 7..0 loading
 * base arg from 0x19851C[D2] and adding Obj_RandRange(4) to its low byte,
 * spawns 0x17A1DE (group 5, count 2), plays hooked sound 0x3A, and jumps to
 * 0x17A84C (+0x06 = 2, next tick 0x17A85C). */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_shared_ext.h"

int Fn_1984C0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t d1 = r->d[1];

    uint32_t a4 = rd32(m, obj + 0x08u);                 /* 0x1984C0 */
    wr16(m, obj + 0x50u, rd16(m, a4 + 0x50u));          /* 0x1984C4 */
    wr16(m, obj + 0x54u, rd16(m, a4 + 0x54u));          /* 0x1984CA */

    for (int16_t d2w = 7; d2w >= 0; d2w--) {            /* 0x1984D0..0x1984FE */
        d1 = rd32(m, 0x0019851Cu + (uint32_t)d2w * 4u); /* 0x1984D4 */
        uint16_t rng = Obj_RandRange(m, 4u);            /* 0x1984DA..0x1984DC */
        d1 = (d1 & 0xFFFFFF00u) | (uint8_t)((uint8_t)d1 + (uint8_t)rng); /* 0x1984E2 */
        d0 = Obj_Spawn(m, obj, 0x0017A1DEu, 5u, 2u, d1); /* 0x1984E4..0x1984FA */
    }

    Sound_PlayHooked(m, 0x003Au);                       /* 0x198502..0x198512 */
    wr16(m, obj + 0x06u, 2u);                           /* 0x198516 -> 0x17A84C */
    wr32(m, obj, 0x0017A85Cu);                          /* 0x17A854 */

    r->d[0] = d0;
    r->d[1] = d1;
    r->d[2] = (r->d[2] & 0xFFFF0000u) | 0xFFFFu;
    r->a[4] = a4;
    return 0;
}
