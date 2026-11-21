#include "obj_CC0F4.h"
#include "../objects/obj_pool.h"

/* ROM 0x0CC0F4 -> 0x0CC0F8 */
int Obj_CC0F4_Tick(const f3mem_t *m, uint32_t obj)
{
    /* ROM 0x0CC0F8 */
    uint32_t a1 = rd32(m, obj + 0x08u);
    wr16(m, a1 + 0x54u, (uint16_t)(rd16(m, a1 + 0x54u) - 1u));
    Obj_Free(m, obj);
    return 0;
}
