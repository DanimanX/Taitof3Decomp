#include "obj_17C4D4.h"

/* ROM 0x17C4D4: install the movement script handed over in +0xC on the
 * target object +0x8: adopt its palette (+0x22), copy the 8-entry
 * {+0x50.w, +0x54.w, +0x10.l} table from +0x10, then idle on 0x17C508 */
int Obj_17C4D4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a4 = rd32(m, obj + 0x08u);
    uint16_t d0 = (uint16_t)rd32(m, obj + 0x0Cu);
    wr16(m, a4 + 0x22u, d0);
    uint32_t a1 = obj + 0x10u;
    wr32(m, a4 + 0x66u, a1);
    wr16(m, a4 + 0x6Au, 0u);
    for (int16_t i = 7; i >= 0; i--) {
        wr16(m, a1, rd16(m, a4 + 0x50u));
        a1 += 2u;
        wr16(m, a1, rd16(m, a4 + 0x54u));
        a1 += 2u;
        wr32(m, a1, rd32(m, a4 + 0x10u));
        a1 += 4u;
    }
    wr32(m, obj + 0x00u, 0x0017C508u);      /* 0x17C500 */
    return 0;
}
