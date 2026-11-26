/* ROM 0x1127C0: enemy wave/variant initializer (A0 = object).
 * Indexes table 0x1127D6 by (+0x0C & 3), resets object fields via 0x0CBD86,
 * loads (+0x44, +0x18, +0x14) from the 3-long record, adds (Rng_NextMem & 0xFF)
 * << 16 to +0x14, and transitions to 0x112848. */
#include "fn_auto.h"
#include "../objects/obj_common.h"

int Fn_1127C0(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint16_t idx = (uint16_t)(rd32(m, obj + 0x0Cu) & 3u); /* 0x1127C0..0x1127C4 */
    uint32_t a1 = rd32(m, 0x001127D6u + (uint32_t)idx * 4u); /* 0x1127C8..0x1127CE */

    Obj_ResetFields(m, obj);                             /* 0x112810 -> 0x0CBD86 */

    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;         /* 0x11281A */
    wr32(m, obj + 0x18u, rd32(m, a1)); a1 += 4u;         /* 0x11281E */
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;         /* 0x112822 */
    r->a[1] = a1;

    uint32_t d0 = (Rng_NextMem(m) & 0x000000FFu) << 16;  /* 0x11282A..0x112836 */
    r->d[0] = d0;
    wr32(m, obj + 0x14u, rd32(m, obj + 0x14u) + d0);     /* 0x112838 */
    wr32(m, obj + 0x00u, 0x00112848u);                   /* 0x112840 */
    return 0;
}
