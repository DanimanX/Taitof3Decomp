#include "obj_17C9D0.h"
#include "obj_17C908.h"
#include "../objects/obj_pool.h"

/* ROM 0x17C9D0: while the +0x3E lifespan timer is still >= 0 run the
 * 0x17C97A body again (follow the parent, animate, draw, re-arm); when it
 * runs out release the object (0x17C9D6: jsr 0x105EC). */
int Obj_17C9D0_Tick(const f3mem_t *m, uint32_t obj)
{
    if ((int16_t)rd16(m, obj + 0x3Eu) >= 0)              /* bge.b 0x17C97A */
        return Obj_17C97A_Body(m, obj);
    Obj_Free(m, obj);
    return 0;
}
