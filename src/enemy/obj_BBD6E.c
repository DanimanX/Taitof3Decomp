#include "obj_BBD6E.h"
#include "../objects/obj_common.h"

/* ROM 0x0BBD6E: init from the table at 0x0BBF2C */
int Obj_BBD6E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a1 = 0x000BBF2Cu;
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x006Fu);
    wr32(m, obj + 0x00u, 0x000BBD94u);
    return 0;
}
