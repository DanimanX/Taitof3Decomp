#include "obj_CC756.h"

/* ROM 0x0CBEB0: speed scaled by the difficulty level (A5+0x87E) */
static uint16_t sub_0CBEB0(const f3mem_t *m, uint16_t d0)
{
    uint16_t d1 = (uint16_t)((uint16_t)(rd16(m, F3_A5 + 0x87Eu) << 3) + 0x0C00u);
    uint32_t p = (uint32_t)((int32_t)(int16_t)d0 * (int32_t)(int16_t)d1);
    p <<= 4;
    return (uint16_t)(p >> 16);
}

/* ROM 0x0CCC08: apply the spawn argument (+0x0C) offsets and angle,
 * place at the parent's position */
static void sub_0CCC08(const f3mem_t *m, uint32_t obj)
{
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(rd16(m, obj + 0x50u) + (uint16_t)(int16_t)(int8_t)(d0 >> 24)));
    wr16(m, obj + 0x52u, (uint16_t)(rd16(m, obj + 0x52u) + (uint16_t)(int16_t)(int8_t)(d0 >> 16)));
    wr16(m, obj + 0x20u, (uint16_t)((d0 >> 8) & 0xFFu));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x34u, 0u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
}

/* ROM 0x0CC756: spinning shot init */
int Obj_CC756_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 4u);
    wr16(m, obj + 0x2Eu, 0x0Bu);
    wr16(m, obj + 0x30u, 4u);
    wr16(m, obj + 0x32u, 0x0Bu);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x50u, 0xFFF8u);
    wr16(m, obj + 0x52u, 0xFFF8u);
    sub_0CCC08(m, obj);
    wr32(m, obj + 0x24u, 1u);
    wr16(m, obj + 0x3Au, (uint16_t)(sub_0CBEB0(m, 4u) + 2u));
    /* 0x0CC79E */
    uint16_t d0 = rd16(m, obj + 0x20u);
    d0 = (uint16_t)((d0 & 0xFF00u) | (uint8_t)(d0 + 4u));
    d0 = (uint16_t)(d0 >> 3);
    wr32(m, obj + 0x10u, rd32(m, 0x000CC8C4u + (uint32_t)(int32_t)(int16_t)d0 * 4u));
    wr16(m, obj + 0x06u, 0x0026u);
    wr32(m, obj + 0x00u, 0x000CC7C0u);
    return 0;
}
