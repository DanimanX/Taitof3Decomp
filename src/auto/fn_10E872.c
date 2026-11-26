/* ROM 0x10E872: falling debris/rock initializer (A0 = object).
 * Seeds object from parameter block 0x10C8E2 via 0x0CEC4C, picks one of 8
 * behaviour descriptors from 0x10E8E6 via Obj_Next4Way (0x0D0E3A) & 7 and
 * installs it via 0x0D0678, jitters X (+0x14) by (Obj_Next4Way & 0x3F) - 0x20,
 * sets colour override (+0x1D) = 0x5C, and arms 0x10E8B4. */
#include "fn_auto.h"
#include "../objects/obj_shared.h"
#include "../objects/obj_shared_ext.h"

int Fn_10E872(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];

    r->a[2] = Obj_SeedFromParams(m, obj, 0x0010C8E2u);    /* 0x10E872..0x10E876 */

    uint16_t idx = (uint16_t)(Obj_Next4Way(m) & 0x0007u); /* 0x10E87C..0x10E882 */
    uint32_t a1 = rd32(m, 0x0010E8E6u + (uint32_t)idx * 4u); /* 0x10E886 */
    Obj_StartBehaviour(m, obj, a1);                       /* 0x10E88E -> 0x0D0678 */
    r->a[1] = a1 + 8u;

    uint16_t dx = (uint16_t)((Obj_Next4Way(m) & 0x003Fu) - 0x0020u); /* 0x10E894..0x10E89E */
    r->d[0] = (r->d[0] & 0xFFFF0000u) | dx;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + dx)); /* 0x10E8A2 */
    wr8(m, obj + 0x1Du, 0x5Cu);                           /* 0x10E8A6 */
    wr32(m, obj, 0x0010E8B4u);                            /* 0x10E8AC */
    return 0;
}
