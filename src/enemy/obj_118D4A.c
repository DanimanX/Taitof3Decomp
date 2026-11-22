#include "obj_118D4A.h"

/* ROM 0x118D4A: post the same 0x118D56 return address to +0x3A, consume the
 * next timer word from the argument stream (+0x0C), then run LAB_00118d74,
 * which re-points the tick vector at 0x118D7C. */
int Obj_118D4A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Au, 0x00118D56u);      /* move.l #0x118d56,(0x3a,A0) */

    /* 0x118D52: bra.w 0x118D66 */
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t d0 = rd16(m, a1);
    a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x06u, d0);

    /* LAB_00118d74 */
    wr32(m, obj + 0x00u, 0x00118D7Cu);
    return 0;
}
