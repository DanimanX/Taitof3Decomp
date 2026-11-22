#include "obj_1795CA.h"

/* Animation engine of the 0x179A5C table, op 13 (tick):
 *   A1 = (0x3A,A0)
 *   (0x3E,A0).w = (A1)+     frame timer
 *   (0x10,A0).l = (A1)+     sprite def
 *   D0w         = (A1)+     flip mode
 *   (0x1C,A0).b = D0b
 *   (0x2C,A0).l = (A1)+     hit box top-left
 *   (0x30,A0).l = (A1)+     hit box bottom-right
 *   D0 = bfexts (A1){0:8}   D1 = bfexts (A1){8:8}   (signed bytes dx, dy)
 *   (0x14,A0) += D0w        (0x18,A0) += D1w        frame offset
 * The record bytes are fetched sign-extended from the ROM image, so the
 * offsets are (int8_t). */
int Obj_1795CA_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1)));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 1u)));
    return 0;
}
