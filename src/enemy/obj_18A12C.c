#include "obj_18A12C.h"
#include "../objects/obj_common.h"
#include "../objects/obj_pool.h"

/* ROM 0x18A12C: countdown +0x06, then copy palette + queue ramp and free */
int Obj_18A12C_Tick(const f3mem_t *m, uint32_t obj)
{
    int16_t t = (int16_t)(rd16(m, obj + 0x06u) - 1u);
    wr16(m, obj + 0x06u, (uint16_t)t);
    if (t > 0) {
        wr32(m, obj + 0x00u, 0x0018A12Cu);
        return 0;
    }
    Mem_CopyLongs(m, 0x0014E21Au, 0x0040B42Eu, 0x000Fu);
    Pal_QueueRamp(m, 0x0014E15Au, 0x0040B42Eu, 0x0B80u, 0x0040u, 0x0010u);
    Obj_Free(m, obj);
    return 0;
}
