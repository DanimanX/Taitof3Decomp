#include "obj_184DA8.h"
#include "../objects/obj_pool.h"

/* ROM 0x184DA0: re-arm the tick vector with 0x184DA8. */
static void Obj_184DA8_Keep(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj, 0x184DA8u);
}

/* ROM 0x184DA8: wait +0x6 frames, then free the object. */
int Obj_184DA8_Tick(const f3mem_t *m, uint32_t obj)
{
    uint16_t d = (uint16_t)(rd16(m, obj + 0x6u) - 1u);
    wr16(m, obj + 0x6u, d);
    if ((int16_t)d > 0) {
        Obj_184DA8_Keep(m, obj);
        return 0;
    }
    Obj_Free(m, obj); /* ROM 0x0105EC */
    return 0;
}
