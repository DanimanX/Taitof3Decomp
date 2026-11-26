#include "obj_119974.h"
#include "../objects/obj_common.h"

/* ROM 0x119974: reset the motion fields, take sprite 0x04F2BC in colour
 * 0x16, arm 0x119990. */
int Obj_119974_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_ResetFields(m, obj);                        /* 0x0CBD86 */
    wr32(m, obj + 0x10u, 0x0004F2BCu);
    wr8(m, obj + 0x1Du, 0x16u);
    wr32(m, obj + 0x00u, 0x00119990u);
    return 0;
}
