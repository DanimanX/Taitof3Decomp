#include "obj_BC540.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"

/* ROM 0x0CBD86: clear the per-frame sprite state */
static void sub_0CBD86(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x1Cu, 0u);
    wr16(m, obj + 0x1Eu, 0u);
    wr16(m, obj + 0x34u, 0u);
    wr16(m, obj + 0x3Au, 0u);
    wr16(m, obj + 0x3Eu, 0u);
    wr32(m, obj + 0x24u, 0u);
    wr32(m, obj + 0x28u, 0u);
}

/* ROM 0xBC540 */
int Obj_BC540_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC540: lea (0x8,PC),A1 -> 0x0BC540 */
    uint32_t a1 = 0x000BC542u + 0x08u;
    sub_0CBD86(m, obj);
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x48u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x0086u);
    wr32(m, obj, 0x000BC576u);
    return 0;
}
