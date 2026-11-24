#include "obj_17C8CC.h"
#include "../objects/obj_pool.h"

/* ROM 0x17C8CC: tail of the 0x17C88E state. While the +0x3E lifespan timer is
 * still positive the state falls back to the 0x17C9D0 body, which the capture
 * never reaches; when it runs out the object is released. */
int Obj_17C8CC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        /* 0x17C8D0 bge.b -> 0x17C97A body, outside this entry's window */
        return -1;
    }
    Obj_Free(m, obj);
    return 0;
}
