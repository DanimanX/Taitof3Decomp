#include "obj_BB86A.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBB86A */
int Obj_BB86A_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BB86A: lea (0x66,PC),A1 -> 0x0BB8D2 (operand at 0x0BB86C),
     * then jump into the shared body at 0x0BB90C */
    uint32_t a1 = 0x000BB8D2u;
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);

    /* 0x0BB91A */
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
