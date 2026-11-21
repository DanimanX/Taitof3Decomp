#include "obj_BDDE2.h"

/* ROM 0x0BDDE2 */
int Obj_BDDE2_Tick(const f3mem_t *m, uint32_t obj)
{
    /* jsr 0x176198 (fill line RAM 0x62B600 with 0x300C) only writes line RAM */

    /* jsr 0x16AB2C: D0 = (A5-0x5DA0), D1 = (A5-0x5D9C) -> A5+0x421C and A5-0x5D90 */
    uint32_t d0 = rd32(m, F3_A5 - 0x5DA0u);
    uint32_t d1 = rd32(m, F3_A5 - 0x5D9Cu);
    wr32(m, F3_A5 + 0x421Cu, d0);
    wr32(m, F3_A5 + 0x4220u, d1);
    wr32(m, F3_A5 - 0x5D90u, d0);
    wr32(m, F3_A5 - 0x5D8Cu, d1);

    /* jsr 0x16AB26: A5+0x423C = {0, 0} */
    wr32(m, F3_A5 + 0x423Cu, 0u);
    wr32(m, F3_A5 + 0x4240u, 0u);

    wr16(m, obj + 0x06u, 0x0005u); /* Z=0: beq 0xBDE48 never taken */
    wr32(m, obj + 0x00u, 0x000BDE42u);
    return 0;
}
