#include "obj_CC05A.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CC05A -> 0x0CC0F8: decrement the parent's counter at +0x54, free self */
int Obj_CC05A_Tick(const f3mem_t *m, uint32_t obj)
{
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x54u, (uint16_t)(rd16(m, a1 + 0x54u) - 1u));
    Obj_Free(m, obj);
    return 0;
}
