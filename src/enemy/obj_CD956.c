#include "obj_CD956.h"

/* ROM 0x0CBEB0: speed scaled by the difficulty level (A5+0x87E) */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x87Eu) << 3) + 0x0C00u);
    uint32_t p = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)(int16_t)d1);
    p <<= 4;
    return (uint16_t)(p >> 16);
}

/* ROM 0x0CD956: homing shot init (target = A5+0x1D28) */
int Obj_CD956_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, a1 + 0x14u) - 8u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, a1 + 0x18u) - 8u));
    uint32_t c = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)(c >> 24)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)(c >> 16)));
    wr16(m, obj + 0x20u, (uint16_t)((c >> 8) & 0xFFu));
    wr32(m, obj + 0x0Cu, rd32(m, F3_A5 + 0x1D28u));
    wr16(m, obj + 0x2Cu, 0u);
    wr16(m, obj + 0x30u, 6u);
    wr16(m, obj + 0x2Eu, 0x0Fu);
    wr16(m, obj + 0x32u, 9u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr16(m, obj + 0x42u, sub_0CBEB0(m, 0x16u));
    wr16(m, obj + 0x4Cu, 0u);
    wr16(m, obj + 0x54u, 1u);
    wr8(m, obj + 0x1Du, 8u);
    wr32(m, obj + 0x10u, 0x0005A5C8u);
    wr32(m, obj + 0x00u, 0x000CD9F6u);
    return 0;
}
