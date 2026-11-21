#include "obj_118CF6.h"

/* ROM 0x118D66: consume one word from A0+0x0C into the countdown, then run
 * the 4-way strip animation starting at 0x118D7C */
static void sub_118D66(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x0Cu);
    uint16_t d0w = rd16(m, a1);
    a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);
    wr16(m, obj + 0x06u, d0w);

    /* 0x118D74 */
    wr32(m, obj + 0x00u, 0x00118D7Cu);
}

/* ROM 0x118CF6 */
int Obj_118CF6_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x3Au, 0x00118D02u);
    sub_118D66(m, obj);   /* 0x118CFE: bra.w 0x118D66 */
    return 0;
}
