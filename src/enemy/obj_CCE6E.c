#include "obj_CCE6E.h"

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

/* ROM 0x0CCBC0: read the bullet record at A1, then 0x0CCC08 */
static void sub_0CCBC0(const f3mem_t *m, uint32_t obj, uint32_t a1)
{
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Cu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x30u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x2Eu, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x32u, (uint16_t)(int16_t)(int8_t)rd8(m, a1++));
    wr16(m, obj + 0x56u, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x58u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x5Cu, rd8(m, a1++));
    wr16(m, obj + 0x54u, (uint16_t)(int16_t)(int8_t)rd8(m, a1));
    sub_0CCC08(m, obj);
}

/* ROM 0x0CCE6E: bullet init, next state 0x0CD256 */
int Obj_CCE6E_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CCBC0(m, obj, 0x000CCEA0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr8(m, obj + 0x1Du, 0x22u);
    wr16(m, obj + 0x1Eu, 0u);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr16(m, obj + 0x42u, (uint16_t)(sub_0CBEB0(m, 0x10u) + 0u));
    wr8(m, obj + 0x4Fu, rd8(m, obj + 0x1Du));
    wr32(m, obj + 0x00u, 0x000CD256u);
    return 0;
}
