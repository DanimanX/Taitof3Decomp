#include "obj_183FB0.h"
#include "../objects/obj_pool.h"

/* ROM 0x183FA8: re-arm the tick vector with 0x183FB0. */
static void Obj_183FB0_Keep(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x183FB0u);
}

/* ROM 0x183FB0: wait +0x6 frames, then free the object. */
int Obj_183FB0_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_183FB0_Keep(m, obj);
        return 0;
    }
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
