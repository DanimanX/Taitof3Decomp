#include "obj_BBD24.h"
#include "../objects/obj_common.h"

/* ROM 0x0BBD24: init from the table at 0x0BBF20 */
int Obj_BBD24_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);
    uint32_t a1 = 0x000BBF20u;
    wr32(m, obj + 0x44u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x14u, rd32(m, a1)); a1 += 4u;
    wr32(m, obj + 0x18u, rd32(m, a1));
    wr16(m, obj + 0x60u, 0x006Fu);
    wr32(m, obj + 0x00u, 0x000BBD4Au);
    return 0;
}
