/* ROM 0x102310: random-sprite effect/spark initializer (A0 = object).
 * Seeds from parameter block 0x1086FC via 0x0CEC4C, saves +0x10 into +0x3E,
 * picks one of four behaviour tables at 0x10238C via 0x0D0E3A and installs it
 * via 0x0D0678, jitters X (+0x14) by (Rng_Mix1D60 & 0x1F) - 0x10, sets +0x1D.b
 * = 0xC0, and transitions to 0x102358. */
#include "fn_auto.h"
#include "../enemy/obj_E8B72.h"
#include "../objects/obj_shared_ext.h"

int Fn_102310(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    uint32_t a2 = Obj_SeedFromParams(m, obj, 0x001086FCu); /* 0x102310..0x102314 */
    wr32(m, obj + 0x3Eu, rd32(m, obj + 0x10u));          /* 0x10231A */

    uint16_t idx = (uint16_t)(Rng_Mix1D60(m) & 0x0003u); /* 0x102320..0x102326 */
    uint32_t a1 = rd32(m, 0x0010238Cu + (uint32_t)idx * 4u); /* 0x10232A */
    Obj_StartBehaviour(m, obj, a1);                      /* 0x102332 */
    a1 += 8u;                                            /* 0x0D0678 leaves A1 at base+8 */

    uint16_t d0w = (uint16_t)((Rng_Mix1D60(m) & 0x001Fu) - 0x0010u); /* 0x102338..0x102342 */
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0w)); /* 0x102346 */
    wr8(m, obj + 0x1Du, 0xC0u);                          /* 0x10234A */
    wr32(m, obj, 0x00102358u);                           /* 0x102350 */

    r->d[0] = (r->d[0] & 0xFFFF0000u) | d0w;
    r->a[1] = a1;
    r->a[2] = a2;
    return 0;
}
