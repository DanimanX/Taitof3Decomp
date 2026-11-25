#include "obj_127560.h"
#include "../objects/obj_common.h"
#include "../objects/obj_shared_ext.h"

/* ROM 0x127560: reset enemy fields, unpack pos/hitbox from +0x0C, arm 0x127584 */
int Obj_127560_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_11B2F6(m, obj);
    uint32_t a1 = Obj_InitPosBox(m, obj, rd32(m, obj + 0x0Cu));
    wr32(m, obj + 0x0Cu, a1);
    wr8(m, obj + 0x34u, 0x00u);
    wr8(m, obj + 0x35u, 0xFFu);
    wr32(m, obj + 0x00u, 0x00127584u);
    return 0;
}
