#include "obj_187024.h"

/* ROM 0x187024: install 0x18702C tick. */
int Obj_187024_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x0018702Cu);
    return 0;
}
