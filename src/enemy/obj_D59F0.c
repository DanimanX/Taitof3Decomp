#include "obj_D59F0.h"
#include "../objects/obj_pool.h"
#include "../objects/obj_common.h"
#include "../video/sprite.h"


/* ROM 0x0CBF7A */
static void sub_0CBF7A(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x10u, 0x28740u);
    wr32(m, obj, 0xCBF8Au);
}

/* ROM 0x0D59F0: delay on +6, then hand off */
int Obj_D59F0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t t = (uint16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, t);
    if ((int16_t)t > 0) {
        wr32(m, obj, 0xD59F0u);
        return 0;
    }
    wr32(m, obj + 0x50u, 0xD5A04u);
    sub_0CBF7A(m, obj);
    return 0;
}
