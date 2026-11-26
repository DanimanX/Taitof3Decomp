#include "obj_119CAC.h"
#include "../objects/obj_common.h"

/* ROM 0x119CAC: reset the motion fields and arm 0x119CBA. */
int Obj_119CAC_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);                        /* 0x0CBD86 */
    wr32(m, obj + 0x00u, 0x00119CBAu);
    return 0;
}
