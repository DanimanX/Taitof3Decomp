#include "obj_BC152.h"
#include "../objects/obj_pool.h"

/* ROM 0xBC152 */
int Obj_BC152_Tick(const f3mem_t *m, uint32_t obj)
{
    /* 0x0BC152 */
    wr16(m, obj + 0x44u, 0x0004u);
    wr16(m, obj + 0x48u, 0x0004u);
    wr16(m, obj + 0x60u, 0x007Fu);
    /* 0x0BC164: move.l #0xBC16C,(A0) */
    wr32(m, obj, 0x000BC16Cu);
    return 0;
}
