#include "obj_130044.h"

/* ROM 0x130044: init — clear position fields, chain to 0x130054 */
int Obj_130044_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x14u, 0u);
    wr16(m, obj + 0x18u, 0u);
    wr32(m, obj, 0x00130054u);
    return 0;
}
