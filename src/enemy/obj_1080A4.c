#include "obj_1080A4.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1080A4: init sub-part from +0x50 parent, arm 0x1080F2 */
int Obj_1080A4_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x50u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) + 5u));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) - 0x0023u));
    Obj_0CEDE4(m, obj, 0x00107FE4u);
    wr32(m, obj + 0x58u, 0x00870000u);
    Obj_0CFA7C(m, obj, 0x00108004u, rd16(m, obj + 0x58u));
    wr8(m, obj + 0x06u, 1u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr32(m, obj + 0x00u, 0x001080F2u);
    return 0;
}
