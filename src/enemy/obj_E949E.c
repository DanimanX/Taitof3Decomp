#include "obj_E949E.h"
#include "obj_E8B72.h"

/* ROM 0x0E949E */
int Obj_E949E_Tick(const f3mem_t *m, uint32_t obj)
{
    Obj_InitMover(m, obj, 0x000E9502u);
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) & ~0x02u));
    wr8(m, obj + 0x34u, (uint8_t)(rd8(m, obj + 0x34u) | 0x04u));
    wr32(m, obj + 0x00u, 0x000E94BCu);
    return 0;
}
