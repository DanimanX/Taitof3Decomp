#include "obj_17955E.h"

/* ROM 0x17955E: frame-script INIT for op 8 (table 0x179A5C index 8). Record
 * at +0x3A: dur.w, spr.l, 1c.w (the low byte is the sprite/plane byte). The
 * timer is loaded, not armed, so the tick releases it on its first pass. */
int Obj_17955E_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1));
    return 0;
}
