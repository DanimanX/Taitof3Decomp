#include "obj_FEE74.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x0FEE74: seed from params, copy +0x10/+0x36 into state block, arm 0x0FEEA0 */
int Obj_FEE74_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a2 = Obj_SeedFromParams(m, obj, 0x000FEF12u);
    wr16(m, a2 + 0u, rd16(m, obj + 0x10u));
    wr32(m, a2 + 4u, rd32(m, obj + 0x36u));
    uint32_t a1 = 0x000FEF4Au;
    wr8(m, obj + 0x1Cu, (uint8_t)rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x68u, rd32(m, a1));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x08u));
    wr32(m, obj + 0x00u, 0x000FEEA0u);
    return 0;
}
