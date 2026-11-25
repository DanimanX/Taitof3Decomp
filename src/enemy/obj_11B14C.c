#include "obj_11B14C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x11B14C: reset enemy fields, unpack pos/hitbox/hp/score, arm 0x11B17C */
int Obj_11B14C_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_11B2F6(m, obj);
    uint32_t a1 = Obj_InitPosBox(m, obj, rd32(m, obj + 0x0Cu));
    wr16(m, obj + 0x24u, rd16(m, a1)); a1 += 2u;
    wr16(m, obj + 0x4Cu, rd16(m, a1)); a1 += 2u;
    wr32(m, obj + 0x0Cu, a1);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x14u));
    wr32(m, obj + 0x00u, 0x0011B17Cu);
    return 0;
}
