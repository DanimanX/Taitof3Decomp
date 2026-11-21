#include "obj_BC07E.h"

/* ROM 0x0BC07E */
int Obj_BC07E_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x60u, 0x007Fu);
    wr32(m, obj + 0x00u, 0x000BC08Cu);
    return 0;
}
