/* ROM 0x0F4846: extending beam/arm segment initializer (A0 = object).
 * Seeds object from 0x0F49DE (0x0CEC4C) and arms 0x0F4858. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

int Fn_0F4846(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t a1 = 0x000F49DEu;                          /* 0x0F4846 */

    uint32_t a2 = Obj_SeedFromParams(m, obj, a1);       /* 0x0F484A -> 0x0CEC4C */

    uint16_t idx = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        idx = (uint16_t)(idx + 0x20u);
    r->d[0] = (r->d[0] & 0xFFFF0000u) | idx;
    r->a[1] = a1 + 0x34u;
    r->a[2] = a2;

    wr32(m, obj, 0x000F4858u);                          /* 0x0F4850 */
    return 0;
}
