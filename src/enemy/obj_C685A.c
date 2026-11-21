#include "obj_C685A.h"
#include "../objects/obj_common.h"

/* ROM 0x0C685A: player-tick init hook - clear +0x06 and enter 0x0C6866. */
int Obj_C685A_Tick(const f3mem_t *m, uint32_t obj)
{
    wr16(m, obj + 0x06u, 0u);
    wr32(m, obj + 0x00u, 0x000C6866u);
    return 0;
}
