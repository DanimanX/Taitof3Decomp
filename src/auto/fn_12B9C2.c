#include "fn_auto.h"
/* ROM 0x12B9C2: Initialize a child object from its parent's position, bounds and heading. */

int Fn_12B9C2(const f3mem_t *m, f3regs_t *r)
{
    uint32_t obj = r->a[0];
    uint32_t parent = rd32(m, obj + 8u);
    r->a[1] = parent;
    wr32(m, obj + 0x14u, rd32(m, parent + 0x14u)); /* 0x12B9C6 */
    wr32(m, obj + 0x18u, rd32(m, parent + 0x18u));
    wr32(m, obj + 0x2Cu, rd32(m, parent + 0x2Cu));
    wr32(m, obj + 0x30u, rd32(m, parent + 0x30u));
    wr16(m, obj + 0x20u, rd16(m, parent + 0x20u));
    wr16(m, obj + 0x1Eu, 0);                       /* 0x12B9E4 */
    wr16(m, obj + 0x1Cu, 0);
    wr32(m, obj + 0x24u, 0x00640000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x42u, 0xEu);
    wr16(m, obj + 0x34u, 0);
    wr8(m, obj + 6u, 0);                           /* 0x12BA06 SF */
    wr32(m, obj, 0x12BA12u);
    r->a[7] += 4u;
    return 0;
}
