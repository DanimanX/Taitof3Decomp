#include "obj_BB856.h"
#include "../objects/obj_common.h"

/* ROM 0x0BB856: lea (0x42,PC) at 0x0BB856 -> 0x0BB89A, then 0x0BB90C */
int Obj_BB856_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = 0x000BB89Au;
    Obj_ResetFields(m, obj);                     /* 0x0BB910 */
    wr32(m, obj + 0x0Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x48u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x4Cu, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x50u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x54u, rd32(m, a1)); a1 += 4u;
    wr8(m, obj + 0x1Fu, rd8(m, a1)); a1 += 1u;
    wr8(m, obj + 0x1Eu, rd8(m, a1)); a1 += 1u;
    wr16(m, obj + 0x58u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x5Cu, rd16(m, obj + 0x58u));
    wr16(m, obj + 0x60u, 0x00B3u);

    /* 0x0BB94A */
    wr32(m, obj, 0x000BB952u);
    return 0;
}
