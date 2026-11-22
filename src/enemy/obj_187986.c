#include "obj_187986.h"
#include "../objects/obj_pool.h"

/* ROM 0x18797E: re-arm the tick vector with 0x187986. */
static void Obj_187986_Keep(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x187986u);
}

/* ROM 0x187986: wait +0x6 frames, then free the object. */
int Obj_187986_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_187986_Keep(m, obj);
        return 0;
    }
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
