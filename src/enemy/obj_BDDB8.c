#include "obj_BDDB8.h"

/* ROM 0x0059B4: clear playfield/line RAM blocks and scroll/clip state. */
static void sub_0059B4(const f3mem_t *m)
{
    for (uint32_t i = 0u; i < 0x400u; i++)
        wr32(m, 0x00613000u + 4u * i, 0u);
    for (uint32_t i = 0u; i < 0x100u; i++)
        wr16(m, 0x0062A600u + 2u * i, 0u);
    for (uint32_t i = 0u; i < 0x100u; i++) {
        wr16(m, 0x00628600u + 2u * i, 0x0080u);
        uint32_t a1 = 0x00624600u + 2u * i;
        wr16(m, a1, (uint16_t)(rd16(m, a1) & 0xFE00u));
    }
    wr32(m, F3_A5 - 0x5D90u, 0u);
    wr16(m, F3_A5 - 0x5D8Cu, 0u);
    wr32(m, F3_A5 - 0x5D68u, 0u);
    wr16(m, F3_A5 - 0x5D64u, 0u);
}

/* ROM 0x176198: fill 0x1D0 bytes at base + (int16_t)(A5 - 0x5D56) with repeated fill.w. */
static void sub_176198(const f3mem_t *m, uint32_t base, uint16_t fill)
{
    uint32_t a0 = base + (uint32_t)(int32_t)(int16_t)rd16(m, F3_A5 - 0x5D56u);
    uint32_t val = ((uint32_t)fill << 16) | fill;
    for (uint32_t i = 0u; i < 0x1D0u / 4u; i++)
        wr32(m, a0 + 4u * i, val);
}

/* ROM 0x0BDDB8: sound/visual subsystem reset, then switch to 0xBDDE2. */
int Obj_BDDB8_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0059B4(m);
    sub_176198(m, 0x00628600u, 0x0080u);
    wr32(m, obj + 0x00u, 0x000BDDE2u);
    return 0;
}
