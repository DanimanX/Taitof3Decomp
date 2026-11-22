#include "obj_115092.h"

/* ROM 0x115092: store sub-routine pointer */
int Obj_115092_Tick(const f3mem_t *m, uint32_t obj)
{
    wr32(m, obj + 0x00u, 0x11509Au);
    return 0;
}
