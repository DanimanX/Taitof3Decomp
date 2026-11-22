#include "obj_179600.h"

/* Frame-script op 9 INIT (table 0x179A5C index 9):
 *   0x179600  move.w #1,(0x3e,A0)   ; a fresh frame counter
 *   0x179606  bra.w  0x179898       ; op-9 tick body
 * 0x179898 (init and tick share it): angle byte (+0x20) / 16 picks one of
 * 16 records of 6 bytes: spr.l, 1c.w. */

/* ROM 0x179898: directional record, 6-byte stride, coarse angle */
static void sub_179898(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 8u) >> 4);
    uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * 6u & 0xFFFFu);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
}

/* ROM 0x179600: frame-script init for op 9 (arm the frame timer, show frame) */
int Obj_179600_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);
    sub_179898(m, obj);
    return 0;
}
