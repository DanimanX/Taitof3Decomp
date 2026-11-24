#include "obj_17C9D0.h"
#include "../objects/obj_pool.h"

/* ROM 0x17C9D0: tail of the 0x17C97A state. While the +0x3E lifespan timer is
 * still positive the state falls back to the 0x17C97A body (parent-relative
 * motion plus the 0x179A5C animation step), which is outside this entry's
 * window; when it runs out the object is released. */
int Obj_17C9D0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        /* 0x17C9D4 bge.b -> 0x17C97A body, not decompiled here */
        return -1;
    }
    Obj_Free(m, obj);
    return 0;
}
