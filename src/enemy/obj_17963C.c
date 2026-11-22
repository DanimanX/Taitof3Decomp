#include "obj_17963C.h"

/* ROM 0x17963C: frame-script tick of op 14 (init 0x17961E). A1 = (0x3A,A0), so
 * D0 = (0x20,A0).b; D0 = (D0 + 8) >> 4 selects one of the 32 16-byte
 * {spr.l, 1c.w, 2c.l, 30.l, dx.b, dy.b} records, then:
 *   (0x10,A0).l = (A1)+     (0x1C,A0).b = (A1).w
 *   (0x2C,A0).l = (A1)+     (0x30,A0).l = (A1)+     (0x14,A0) += dx.b
 *   (0x18,A0) += dy.b
 * The record is walked on +0x3A in the object so 0x1795CA / 0x179628 stay
 * free of it; the frame timer +0x3E is left to the caller, as in the ROM. */
int Obj_17963C_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t idx = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + idx * 0x10u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 1u)));
    return 0;
}
