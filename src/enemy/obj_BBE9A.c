#include "obj_BBE9A.h"
#include "../objects/obj_common.h"

/* ROM 0x0BBE9A */
int Obj_BBE9A_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a1 = 0x000BBF50u; /* lea (0xAE,PC) */
    wr32(m, obj + 0x10u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x006Fu);
    wr32(m, obj + 0x00u, 0x000BBEC0u);
    return 0;
}
