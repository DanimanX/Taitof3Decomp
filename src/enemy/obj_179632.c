#include "obj_179632.h"

/* Frame-script op 6 INIT (table 0x179A5C index 6):
 *   0x179632  move.w #1,(0x3e,A0)   ; a fresh frame counter
 *   0x179638  bra.w  0x179946       ; op-6 tick body
 * 0x179946 (init and tick share it): angle byte (+0x20) / 16 picks one of
 * 16 records of 14 bytes: spr.l, box 2c.l, 30.l, (dx.b, dy.b). */

/* ROM 0x179946: directional record, 14-byte stride, coarse angle */
static void sub_179946(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * 0x0Eu & 0xFFFFu);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1)); a1 += 4u;
    d0 = (uint16_t)(int16_t)(int8_t)rd8(m, a1);
    uint16_t d1 = (uint16_t)(int16_t)(int8_t)rd8(m, a1 + 1u);
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + d0));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + d1));
}

/* ROM 0x179632: frame-script init for op 6 (arm the frame timer, show frame) */
int Obj_179632_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);
    sub_179946(m, obj);
    return 0;
}
