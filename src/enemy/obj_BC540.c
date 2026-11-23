#include "obj_BC540.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0xBC540 */
int Obj_BC540_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC540: lea (0x8,PC),A1 -> 0x0BC540 */
    uint32_t a1 = 0x000BC542u + 0x08u;
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x48u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x0086u);
    wr32(m, obj, 0x000BC576u);
    return 0;
}
