#include "obj_1084C0.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x1084C0: init sub-part from 3x +0x54 chain, arm 0x108512 */
int Obj_1084C0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, rd32(m, rd32(m, obj + 0x54u) + 0x54u) + 0x54u);
    wr32(m, obj + 0x14u, rd32(m, a1 + 0x14u));
    wr16(m, obj + 0x14u, (uint16_t)(rd16(m, obj + 0x14u) - 0x001Au));
    wr32(m, obj + 0x18u, rd32(m, a1 + 0x18u));
    wr16(m, obj + 0x18u, (uint16_t)(rd16(m, obj + 0x18u) + 9u));
    Obj_0CEDE4(m, obj, 0x00108400u);
    wr32(m, obj + 0x58u, 0x00870000u);
    Obj_0CFA7C(m, obj, 0x00108420u, rd16(m, obj + 0x58u));
    wr8(m, obj + 0x06u, 1u);
    wr32(m, obj + 0x00u, 0x00108512u);
    return 0;
}
