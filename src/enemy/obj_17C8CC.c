#include "obj_17C8CC.h"
#include "obj_17C856.h"
#include "../objects/obj_pool.h"

/* ROM 0x17C8CC: tail of the 0x17C88E state. While the +0x3E lifespan timer is
 * still >= 0 it runs the 0x17C88E body again; when it runs out the object
 * is released. */
int Obj_17C8CC_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0) {
        return Obj_17C88E_Body(m, obj);             /* 0x17C8D0 bge.b 0x17C88E */
    }
    Obj_Free(m, obj);
    return 0;
}
