#include "obj_17ABC8.h"
#include "../objects/obj_pool.h"

/* ROM 0x17ABC0: re-arm the tick vector with 0x17ABC8. */
static void Obj_17ABC8_Keep(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x17ABC8u);
}

/* ROM 0x17ABC8: wait +0x6 frames, then free the object. */
int Obj_17ABC8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_17ABC8_Keep(m, obj);
        return 0;
    }
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
