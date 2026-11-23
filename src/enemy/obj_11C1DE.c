#include "obj_11C1DE.h"
#include "../objects/obj_common.h"

/* ROM 0x11C1DE: clear 8 longs at 0x40B0F6, init */
int Obj_11C1DE_Tick(const f3mem_t *m, uint32_t obj)
{
    for (uint32_t i = 0; i < 8u; i++)
        wr32(m, 0x0040B0F6u + i * 4u, 0u);
    Obj_ResetFields(m, obj);
    wr32(m, obj + 0x00u, 0x0011C1FCu);
    return 0;
}
