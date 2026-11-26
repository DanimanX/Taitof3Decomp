/* ROM 0x0E18CA: trail/chain spawner initializer (A0 = object).
 * Seeds from 0x0DC84E (0x0CEC4C), sets +0x23 = 7, +0x54 = -1, +0x06 = 1,
 * and arms 0x0E18F0. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0E18CA(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x000DC84Eu;                          /* 0x0E18CA */

    Obj_SeedFromParams(m, obj, a1);                     /* 0x0E18CE -> 0x0CEC4C */

    uint16_t idx = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        idx = (uint16_t)(idx + 0x20u);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | idx;
    r->a[1] = a1 + 0x34u;

    wr8(m, obj + 0x23u, 0x07u);                         /* 0x0E18D4 */
    wr32(m, obj + 0x54u, 0xFFFFFFFFu);                  /* 0x0E18DA */
    wr8(m, obj + 0x06u, 0x01u);                         /* 0x0E18E2 */
    wr32(m, obj, 0x000E18F0u);                          /* 0x0E18E8 */
    return 0;
}
