#include "obj_BBE2A.h"

/* ROM 0x0CBD86: clear position/animation state */
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

/* ROM 0x0BBE2A: init from the table at 0x0BBF44 */
int Obj_BBE2A_Tick(const f3mem_t *m, uint32_t obj)
{
    sub_0CBD86(m, obj);
    uint32_t a1 = 0x000BBF44u;
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x006Fu);
    wr32(m, obj + 0x00u, 0x000BBE50u);
    return 0;
}
