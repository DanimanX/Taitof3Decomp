#include "obj_1899EC.h"
#include "../objects/obj_pool.h"

/* ROM 0x1899E4: re-arm the tick vector with 0x1899EC. */
static void Obj_1899EC_Keep(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x1899ECu);
}

/* ROM 0x1899EC: wait +0x6 frames, then free the object. */
int Obj_1899EC_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_1899EC_Keep(m, obj);
        return 0;
    }
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
