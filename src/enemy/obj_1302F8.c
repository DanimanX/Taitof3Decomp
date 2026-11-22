#include "obj_1302F8.h"
#include "../objects/obj_pool.h"

/* ROM 0x1302F8 */
int Obj_1302F8_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x1302F8: pause/warp flag -> free */
    if ((rd8(m, F3_A5 + 0x30F2u) & 0x01u) != 0u) {
        Obj_Free(m, obj);                           /* 0x130352 */
        return 0;
    }

    /* 0x130302 */
    uint32_t d0 = rd32(m, obj + 0x0Cu);
    wr16(m, obj + 0x06u, (uint16_t)d0);
    wr32(m, obj + 0x00u, 0x00130312u);
    return 0;
}
