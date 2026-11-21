#include "obj_BC1CE.h"

/* ROM 0x0BC1CE: init */
int Obj_BC1CE_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x44u, 0x0004u);
    wr16(m, obj + 0x48u, 0x000Fu);
    wr16(m, obj + 0x60u, 0x006Fu);
    wr32(m, obj + 0x00u, 0x000BC1E8u);
    return 0;
}
