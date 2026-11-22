#include "obj_BDDE2.h"

/* ROM 0x176198: fill 0x1D0 bytes (232 words) of line RAM at dst + (A5-0x5D56) with val */
static void sub_176198(const f3mem_t *m, uint32_t dst, uint16_t val)
{
    uint32_t a0 = dst + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    for (uint32_t off = 0; off < 0x1D0u; off += 2u)
        wr16(m, a0 + off, val);
}

/* ROM 0x0BDDE2 */
int Obj_BDDE2_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_176198(m, 0x0062B600u, 0x300Cu);

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
