#include "obj_113B74.h"


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

/* ROM 0x113B74: hitbox, stats, then wait 0x14 frames in 0x113BC2 */
int Obj_113B74_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x2Cu, 5u);
    wr16(m, obj + 0x30u, 5u);
    wr16(m, obj + 0x2Eu, 0x1Au);
    wr16(m, obj + 0x32u, 0x1Au);
    sub_113652(m, obj);
    wr32(m, obj + 0x24u, 0x00640000u);
    wr32(m, obj + 0x28u, 0x00008000u);
    wr16(m, obj + 0x42u, 0x14u);
    /* sge + bfins {6:1}: bit 1 of +0x1C = (angle byte >= 0) */
    uint8_t b = rd8(m, obj + 0x1Cu);
    if ((int8_t)rd8(m, obj + 0x20u) >= 0)
        b |= 0x02u;
    else
        b &= (uint8_t)~0x02u;
    wr8(m, obj + 0x1Cu, b);
    wr16(m, obj + 0x06u, 0x14u);
    wr32(m, obj + 0x00u, 0x00113BC2u);
    return 0;
}
