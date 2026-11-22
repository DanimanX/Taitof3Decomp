#include "obj_179614.h"

/* Frame-script op 10 INIT (table 0x179A5C index 10):
 *   0x179614  move.w #1,(0x3e,A0)   ; a fresh frame counter
 *   0x17961A  bra.w  0x1798D4       ; op-10 tick body
 * 0x1798D4 (init and tick share it): finer angle, still a 6-byte record:
 * spr.l, 1c.w. */

/* ROM 0x1798D4: directional record, 6-byte stride, fine angle */
static void sub_1798D4(const f3mem_t *m, uint32_t obj)
{
    uint16_t d0 = (uint16_t)((uint8_t)(rd8(m, obj + 0x20u) + 4u) >> 3);
    uint32_t a1 = rd32(m, obj + 0x3Au) + ((uint32_t)d0 * 6u & 0xFFFFu);
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
}

/* ROM 0x179614: frame-script init for op 10 (arm the frame timer, show frame) */
int Obj_179614_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x3Eu, 1u);
    sub_1798D4(m, obj);
    return 0;
}
