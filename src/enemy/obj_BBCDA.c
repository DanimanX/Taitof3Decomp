#include "obj_BBCDA.h"
#include "../objects/obj_common.h"

/* ROM 0x0BBCDA */
int Obj_BBCDA_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a1 = 0x000BBF14u; /* lea (0x232,PC) */
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x007Fu);
    wr32(m, obj + 0x00u, 0x000BBD00u);
    return 0;
}
