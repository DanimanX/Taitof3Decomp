#include "obj_179588.h"

/* ROM 0x179588: frame-script INIT for op 11 (table 0x179A5C index 11). Record
 * at +0x3A: dur.w, spr.l, 1c.w, box 2c.l, 30.l. The timer is loaded, not
 * armed, so the tick releases it on its first pass. */
int Obj_179588_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x3Au);
    wr16(m, obj + 0x3Eu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x2Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x30u, rd32(m, a1));
    return 0;
}
