#include "obj_CD74E.h"

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

/* ROM 0x0CD74E */
int Obj_CD74E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 1u);
    wr16(m, obj + 0x2Eu, 0x0Eu);
    wr16(m, obj + 0x30u, 1u);
    wr16(m, obj + 0x32u, 0x0Eu);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x22u, 0x0A00u);
    wr16(m, obj + 0x42u, 0u);
    wr16(m, obj + 0x50u, 0xFFF8u);
    wr16(m, obj + 0x52u, 0xFFF8u);
    sub_0CCC08(m, obj);
    wr32(m, obj + 0x24u, 0x00010000u);
    wr32(m, obj + 0x28u, 0x27100000u);
    wr8(m, obj + 0x4Fu, 0x1Cu);
    wr8(m, obj + 0x1Du, 0x1Cu);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x02u));
    wr16(m, obj + 0x06u, 0x012Cu);
    wr32(m, obj + 0x00u, 0x000CD7B8u);
    return 0;
}
