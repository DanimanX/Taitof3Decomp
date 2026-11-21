#include "obj_114A10.h"

/* ROM 0x113652: init from the spawn argument (+0xC: dx.b, dy.b, angle.b, flip.b)
 * at the parent's position */
static void sub_113652(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr8(m, obj + 0x4Fu, 0u);
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x50u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 24));
    wr16(m, obj + 0x52u, (uint16_t)(int16_t)(int8_t)(uint8_t)(d0 >> 16));
    wr16(m, obj + 0x20u, (uint16_t)(((d0 >> 8) & 0xFFu) << 8));
    wr16(m, obj + 0x22u, (uint16_t)(d0 & 0xFFu));
    if ((d0 & 0xFFu) != 0u)
        wr8(m, obj + 0x1Cu, (uint8_t)(rd8(m, obj + 0x1Cu) ^ 0x01u));
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + rd16(m, obj + 0x50u)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + rd16(m, obj + 0x52u)));
}

/* ROM 0x114A10: init, targeting a player (+0x22 selects 0x41A988's entry) */
int Obj_114A10_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 0x0008u);
    wr16(m, obj + 0x2Eu, 0x0017u);
    wr16(m, obj + 0x30u, 0x0008u);
    wr16(m, obj + 0x32u, 0x0017u);
    wr32(m, obj + 0x10u, 0x00029F78u);
    sub_113652(m, obj);
    wr32(m, obj + 0x24u, 0x00080000u);
    wr32(m, obj + 0x28u, 0x00010000u);
    wr16(m, obj + 0x42u, 0x0000u);
    uint32_t a1;
    if (rd16(m, obj + 0x22u) == 0u)
        a1 = rd32(m, F3_A5 + 0x1D28u);
    else
        a1 = rd32(m, 0x0041A988u);
    wr32(m, obj + 0x3Au, a1);
    wr16(m, obj + 0x06u, 0x0000u);
    wr32(m, obj + 0x00u, 0x00114A76u);
    return 0;
}
