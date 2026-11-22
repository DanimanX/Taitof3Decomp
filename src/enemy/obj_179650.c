#include "obj_179650.h"

/* ROM 0x179650: frame-script tick of op 15 (init 0x179632), the fine-angle
 * sibling of 0x17963C. Same 16-byte {spr.l, 1c.w, 2c.l, 30.l, dx.b, dy.b}
 * records, but the record is picked from the *finer* 64-step angle
 * (D0 + 4) >> 3 instead of (D0 + 8) >> 4. */
int Obj_179650_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    uint32_t a1 = rd32(m, obj + 0x3Au) + idx * 0x10u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 1u)));
    return 0;
}
