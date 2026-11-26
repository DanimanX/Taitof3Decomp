/* ROM 0x0DF61A: initialize chain spawner object from 0x0DC84E -> 0x0DF640. */
#include "fn_auto.h"
#include "../objects/obj_shared_ext.h"

static uint32_t setw(uint32_t reg, uint16_t w)
{
    return (reg & 0xFFFF0000u) | w;
}

int Fn_0DF61A(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t d0 = r->d[0];
    uint32_t a1 = 0x000DC84Eu;                          /* 0x0DF61A */

    (void)Obj_SeedFromParams(m, obj, a1);               /* 0x0DF61E */
    a1 += 0x34u;
    uint16_t d0w = rd8(m, obj + 0x05u);
    if (rd8(m, obj + 0x04u) != 5u)
        d0w = (uint16_t)(d0w + 0x20u);
    d0 = setw(d0, d0w);

    wr8(m, obj + 0x23u, 7u);                            /* 0x0DF624 */
    wr32(m, obj + 0x54u, 0xFFFFFFFFu);                  /* 0x0DF62A */
    wr8(m, obj + 0x06u, 1u);                            /* 0x0DF632 */
    wr32(m, obj + 0x00u, 0x000DF640u);                  /* 0x0DF638 */

    r->d[0] = d0;
    r->a[1] = a1;
    return 0;
}
