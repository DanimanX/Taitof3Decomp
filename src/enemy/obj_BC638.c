#include "obj_BC638.h"
#include "../objects/obj_pool.h"
/* ROM 0xBC638 */
int Obj_BC638_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC638 */
    wr32(m, F3_A5 + 0x4234u, rd32(m, F3_A5 + 0x4234u) - 0x5C0u);
    wr16(m, obj + 0x60u, (uint16_t)(rd16(m, obj + 0x60u) - 1u));
    if ((int16_t)rd16(m, obj + 0x60u) >= 0) {
        /* 0x0BC630 */
        wr32(m, obj, 0x000BC638u);
        return 0;
    }
    Obj_Free(m, obj);
    return 0;
}
