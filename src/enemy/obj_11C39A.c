#include "obj_11C39A.h"
#include "../objects/obj_common.h"

/* ROM 0x11C39A: copy the global A5+0x30DA (32-bit) into obj+0x24, then
 * continue at 0x11C3A8. */
int Obj_11C39A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x24u, rd32(m, F3_A5 + 0x30DAu));
    wr32(m, obj + 0x00u, 0x0011C3A8u);
    return 0;
}
